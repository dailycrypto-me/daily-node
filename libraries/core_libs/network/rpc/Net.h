#pragma once

#include "NetFace.h"

namespace daily {
class FullNode;
}

namespace daily::net {

class Net : public NetFace {
 public:
  explicit Net(std::shared_ptr<daily::FullNode> const& _full_node);
  virtual RPCModules implementedModules() const override { return RPCModules{RPCModule{"net", "1.0"}}; }
  virtual std::string net_version() override;
  virtual std::string net_peerCount() override;
  virtual bool net_listening() override;

 private:
  std::weak_ptr<daily::FullNode> full_node_;
};

}  // namespace daily::net
