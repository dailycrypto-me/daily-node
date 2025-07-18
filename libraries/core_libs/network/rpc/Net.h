#pragma once

#include "NetFace.h"
#include "common/app_base.hpp"

namespace daily::net {

class Net : public NetFace {
 public:
  explicit Net(std::shared_ptr<daily::AppBase> const& app);
  virtual RPCModules implementedModules() const override { return RPCModules{RPCModule{"net", "1.0"}}; }
  virtual std::string net_version() override;
  virtual std::string net_peerCount() override;
  virtual bool net_listening() override;

 private:
  std::weak_ptr<daily::AppBase> app_;
};

}  // namespace daily::net
