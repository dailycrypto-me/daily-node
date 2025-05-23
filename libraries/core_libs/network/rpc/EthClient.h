/**
 * This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!
 */

#ifndef JSONRPC_CPP_STUB_DAILY_NET_ETHCLIENT_H_
#define JSONRPC_CPP_STUB_DAILY_NET_ETHCLIENT_H_

#include <jsonrpccpp/client.h>

namespace daily {
namespace net {
class EthClient : public jsonrpc::Client {
 public:
  EthClient(jsonrpc::IClientConnector& conn, jsonrpc::clientVersion_t type = jsonrpc::JSONRPC_CLIENT_V2)
      : jsonrpc::Client(conn, type) {}

  std::string eth_protocolVersion() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_protocolVersion", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_coinbase() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_coinbase", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_gasPrice() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_gasPrice", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_accounts() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_accounts", p);
    if (result.isArray())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_blockNumber() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_blockNumber", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_getBalance(const std::string& param1, const Json::Value& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getBalance", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_getStorageAt(const std::string& param1, const std::string& param2,
                               const Json::Value& param3) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    p.append(param3);
    Json::Value result = this->CallMethod("eth_getStorageAt", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_getStorageRoot(const std::string& param1,
                                 const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getStorageRoot", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_getTransactionCount(const std::string& param1,
                                      const Json::Value& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getTransactionCount", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getBlockTransactionCountByHash(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getBlockTransactionCountByHash", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getBlockTransactionCountByNumber(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getBlockTransactionCountByNumber", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getUncleCountByBlockHash(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getUncleCountByBlockHash", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getUncleCountByBlockNumber(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getUncleCountByBlockNumber", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_getCode(const std::string& param1, const Json::Value& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getCode", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_call(const Json::Value& param1, const Json::Value& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_call", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getBlockByHash(const std::string& param1, bool param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getBlockByHash", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getBlockByNumber(const std::string& param1, bool param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getBlockByNumber", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getTransactionByHash(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getTransactionByHash", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getTransactionByBlockHashAndIndex(const std::string& param1,
                                                    const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getTransactionByBlockHashAndIndex", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getTransactionByBlockNumberAndIndex(const std::string& param1,
                                                      const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getTransactionByBlockNumberAndIndex", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getTransactionReceipt(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getTransactionReceipt", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getUncleByBlockHashAndIndex(const std::string& param1,
                                              const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getUncleByBlockHashAndIndex", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getUncleByBlockNumberAndIndex(const std::string& param1,
                                                const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_getUncleByBlockNumberAndIndex", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_newFilter(const Json::Value& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_newFilter", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_newBlockFilter() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_newBlockFilter", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_newPendingTransactionFilter() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_newPendingTransactionFilter", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  bool eth_uninstallFilter(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_uninstallFilter", p);
    if (result.isBool())
      return result.asBool();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getFilterChanges(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getFilterChanges", p);
    if (result.isArray())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getFilterLogs(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getFilterLogs", p);
    if (result.isArray())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getLogs(const Json::Value& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getLogs", p);
    if (result.isArray())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_sendRawTransaction(const std::string& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_sendRawTransaction", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_syncing() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_syncing", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  std::string eth_estimateGas(const Json::Value& param1, const std::string& param2) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    p.append(param2);
    Json::Value result = this->CallMethod("eth_estimateGas", p);
    if (result.isString())
      return result.asString();
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_chainId() throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p = Json::nullValue;
    Json::Value result = this->CallMethod("eth_chainId", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
  Json::Value eth_getBlockReceipts(const Json::Value& param1) throw(jsonrpc::JsonRpcException) {
    Json::Value p;
    p.append(param1);
    Json::Value result = this->CallMethod("eth_getBlockReceipts", p);
    if (result.isObject())
      return result;
    else
      throw jsonrpc::JsonRpcException(jsonrpc::Errors::ERROR_CLIENT_INVALID_RESPONSE, result.toStyledString());
  }
};

}  // namespace net
}  // namespace daily
#endif  // JSONRPC_CPP_STUB_DAILY_NET_ETHCLIENT_H_
