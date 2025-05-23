#include "metrics/metrics_service.hpp"

#include <prometheus/exposer.h>
#include <prometheus/registry.h>

#include <chrono>
#include <memory>
#include <thread>

namespace daily::metrics {
MetricsService::MetricsService(const std::string& host, uint16_t port, uint16_t polling_interval_ms)
    : kPollingIntervalMs(polling_interval_ms) {
  exposer_ = std::make_unique<prometheus::Exposer>(host + ":" + std::to_string(port));
  registry_ = std::make_shared<prometheus::Registry>();
  exposer_->RegisterCollectable(registry_);
}

MetricsService::~MetricsService() {
  if (thread_) {
    thread_->join();
  }
}
void MetricsService::start() {
  if (!exposer_) {
    return;
  }
  thread_ = std::make_unique<std::thread>([&]() {
    for (;;) {
      for (auto& r : metrics_) {
        r.second->updateData();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(kPollingIntervalMs));
    }
  });
}
}  // namespace daily::metrics