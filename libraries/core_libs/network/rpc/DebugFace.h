/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_DAILY_NET_DEBUGFACE_H_
#define JSONRPC_CPP_STUB_DAILY_NET_DEBUGFACE_H_

#include <libweb3jsonrpc/ModularServer.h>

namespace daily {
namespace net {
class DebugFace : public ServerInterface<DebugFace> {
 public:
  DebugFace() {
    this->bindAndAddMethod(jsonrpc::Procedure("debug_traceTransaction", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_OBJECT, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_traceTransactionI);
    this->bindAndAddMethod(jsonrpc::Procedure("debug_traceCall", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_OBJECT,
                                              "param1", jsonrpc::JSON_OBJECT, "param2", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_traceCallI);
    this->bindAndAddMethod(jsonrpc::Procedure("debug_getPreviousBlockCertVotes", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_OBJECT, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_getPreviousBlockCertVotesI);
    this->bindAndAddMethod(jsonrpc::Procedure("debug_getPeriodTransactionsWithReceipts", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_OBJECT, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_getPeriodTransactionsWithReceiptsI);
    this->bindAndAddMethod(jsonrpc::Procedure("debug_getPeriodDagBlocks", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_OBJECT, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_getPeriodDagBlocksI);
    this->bindAndAddMethod(
        jsonrpc::Procedure("trace_call", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_OBJECT, "param1",
                           jsonrpc::JSON_OBJECT, "param2", jsonrpc::JSON_ARRAY, "param3", jsonrpc::JSON_STRING, NULL),
        &daily::net::DebugFace::trace_callI);
    this->bindAndAddMethod(
        jsonrpc::Procedure("trace_replayTransaction", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_OBJECT, "param1",
                           jsonrpc::JSON_STRING, "param2", jsonrpc::JSON_ARRAY, NULL),
        &daily::net::DebugFace::trace_replayTransactionI);
    this->bindAndAddMethod(
        jsonrpc::Procedure("trace_replayBlockTransactions", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_OBJECT, "param1",
                           jsonrpc::JSON_STRING, "param2", jsonrpc::JSON_ARRAY, NULL),
        &daily::net::DebugFace::trace_replayBlockTransactionsI);

    this->bindAndAddMethod(jsonrpc::Procedure("debug_dposValidatorTotalStakes", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_ARRAY, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_dposValidatorTotalStakesI);
    this->bindAndAddMethod(jsonrpc::Procedure("debug_dposTotalAmountDelegated", jsonrpc::PARAMS_BY_POSITION,
                                              jsonrpc::JSON_ARRAY, "param1", jsonrpc::JSON_STRING, NULL),
                           &daily::net::DebugFace::debug_dposTotalAmountDelegatedI);
  }

  inline virtual void debug_traceTransactionI(const Json::Value& request, Json::Value& response) {
    response = this->debug_traceTransaction(request[0u].asString());
  }
  inline virtual void debug_traceCallI(const Json::Value& request, Json::Value& response) {
    response = this->debug_traceCall(request[0u], request[1u].asString());
  }
  inline virtual void debug_getPreviousBlockCertVotesI(const Json::Value& request, Json::Value& response) {
    response = this->debug_getPreviousBlockCertVotes(request[0u].asString());
  }
  inline virtual void debug_getPeriodTransactionsWithReceiptsI(const Json::Value& request, Json::Value& response) {
    response = this->debug_getPeriodTransactionsWithReceipts(request[0u].asString());
  }
  inline virtual void debug_getPeriodDagBlocksI(const Json::Value& request, Json::Value& response) {
    response = this->debug_getPeriodDagBlocks(request[0u].asString());
  }
  inline virtual void trace_callI(const Json::Value& request, Json::Value& response) {
    response = this->trace_call(request[0u], request[1u], request[2u].asString());
  }
  inline virtual void trace_replayTransactionI(const Json::Value& request, Json::Value& response) {
    response = this->trace_replayTransaction(request[0u].asString(), request[1u]);
  }
  inline virtual void trace_replayBlockTransactionsI(const Json::Value& request, Json::Value& response) {
    response = this->trace_replayBlockTransactions(request[0u].asString(), request[1u]);
  }
  inline virtual void debug_dposValidatorTotalStakesI(const Json::Value& request, Json::Value& response) {
    response = this->debug_dposValidatorTotalStakes(request[0u].asString());
  }
  inline virtual void debug_dposTotalAmountDelegatedI(const Json::Value& request, Json::Value& response) {
    response = this->debug_dposTotalAmountDelegated(request[0u].asString());
  }

  virtual Json::Value debug_traceTransaction(const std::string& param1) = 0;
  virtual Json::Value debug_traceCall(const Json::Value& param1, const std::string& param2) = 0;
  virtual Json::Value debug_getPreviousBlockCertVotes(const std::string& param1) = 0;
  virtual Json::Value debug_getPeriodTransactionsWithReceipts(const std::string& param1) = 0;
  virtual Json::Value debug_getPeriodDagBlocks(const std::string& param1) = 0;
  virtual Json::Value trace_call(const Json::Value& param1, const Json::Value& param2, const std::string& param3) = 0;
  virtual Json::Value trace_replayTransaction(const std::string& param1, const Json::Value& param2) = 0;
  virtual Json::Value trace_replayBlockTransactions(const std::string& param1, const Json::Value& param2) = 0;
  virtual Json::Value debug_dposValidatorTotalStakes(const std::string& param1) = 0;
  virtual Json::Value debug_dposTotalAmountDelegated(const std::string& param1) = 0;
};

}  // namespace net
}  // namespace daily
#endif  // JSONRPC_CPP_STUB_DAILY_NET_DEBUGFACE_H_
