#pragma once

#include <jsonrpccpp/common/exception.h>
#include <jsonrpccpp/server.h>
#include <libdevcore/Common.h>

#include <memory>

#include "DailyFace.h"
#include "common/app_base.hpp"
#include "libweb3jsonrpc/ModularServer.h"

namespace daily::net {

class Daily : public DailyFace {
 public:
  explicit Daily(std::shared_ptr<daily::AppBase> app);

  virtual RPCModules implementedModules() const override { return RPCModules{RPCModule{"daily", "1.0"}}; }

  virtual std::string daily_protocolVersion() override;
  virtual Json::Value daily_getVersion() override;
  virtual Json::Value daily_getDagBlockByHash(const std::string& _blockHash, bool _includeTransactions) override;
  virtual Json::Value daily_getDagBlockByLevel(const std::string& _blockLevel, bool _includeTransactions) override;
  virtual std::string daily_dagBlockLevel() override;
  virtual std::string daily_dagBlockPeriod() override;
  virtual Json::Value daily_getScheduleBlockByPeriod(const std::string& _period) override;
  virtual Json::Value daily_getNodeVersions() override;
  virtual std::string daily_pbftBlockHashByPeriod(const std::string& _period) override;
  virtual Json::Value daily_getConfig() override;
  virtual Json::Value daily_getChainStats() override;
  virtual std::string daily_yield(const std::string& _period) override;
  virtual std::string daily_totalSupply(const std::string& _period) override;
  virtual Json::Value daily_getPillarBlockData(const std::string& pillar_block_period,
                                                bool include_signatures) override;

 protected:
  std::weak_ptr<daily::AppBase> app_;

 private:
  Json::Value version;

  std::shared_ptr<daily::AppBase> tryGetApp();
};

}  // namespace daily::net
