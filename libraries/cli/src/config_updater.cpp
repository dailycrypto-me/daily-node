#include "cli/config_updater.hpp"

#include "cli/tools.hpp"

namespace daily::cli {

//// NetworkIPChange
auto NetworkIPChange = [](Json::Value&, const Json::Value&) {
  // if (!old_conf["network_address"].isNull()) {
  //   old_conf["network"]["listen_ip"] = old_conf["network_address"];
  //   old_conf.removeMember("network_address");
  // } else if (old_conf["network"]["listen_ip"].isNull()) {
  //   old_conf["network"]["listen_ip"] = new_conf["network"]["listen_ip"];
  // }
};
//// NetworkIPChange

ConfigUpdater::ConfigUpdater(int chain_id) {
  new_conf_ = tools::getConfig(static_cast<Config::ChainIdType>(chain_id));
  // Register changes that should apply
  config_changes_.emplace_back(NetworkIPChange);
}

void ConfigUpdater::UpdateConfig(Json::Value& old_conf) {
  for (const auto& change : config_changes_) {
    change.apply(old_conf, new_conf_);
  }
}

}  // namespace daily::cli