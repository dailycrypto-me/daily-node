#pragma once

#include <functional>

namespace daily::util {
using task_t = std::function<void()>;
using task_executor_t = std::function<void(task_t &&)>;

inline task_executor_t current_thread_executor() {
  return [](auto &&task) { task(); };
}

}  // namespace daily::util