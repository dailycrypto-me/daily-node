#pragma once

#include <stdexcept>

namespace daily {

struct ConfigException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

}  // namespace daily
