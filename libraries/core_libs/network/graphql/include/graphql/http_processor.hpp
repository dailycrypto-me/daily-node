#pragma once

#include "dag/dag.hpp"
#include "final_chain/final_chain.hpp"
#include "mutation.hpp"
#include "network/http_server.hpp"
#include "network/network.hpp"
#include "query.hpp"
#include "subscription.hpp"
#include "transaction/gas_pricer.hpp"
namespace daily::net {
class GraphQlHttpProcessor final : public HttpProcessor {
 public:
  GraphQlHttpProcessor(std::shared_ptr<::daily::final_chain::FinalChain> final_chain,
                       std::shared_ptr<::daily::DagManager> dag_manager,
                       std::shared_ptr<::daily::PbftManager> pbft_manager,
                       std::shared_ptr<::daily::TransactionManager> transaction_manager,
                       std::shared_ptr<::daily::DbStorage> db, std::shared_ptr<::daily::GasPricer> gas_pricer,
                       std::weak_ptr<::daily::Network> network, uint64_t chain_id);
  Response process(const Request& request) override;

 private:
  Response createErrResponse(std::string&& = "");
  Response createErrResponse(graphql::response::Value&& error_value);
  Response createOkResponse(std::string&& response_body);

 private:
  std::shared_ptr<graphql::daily::Query> query_;
  std::shared_ptr<graphql::daily::Mutation> mutation_;
  std::shared_ptr<graphql::daily::Subscription> subscription_;
  graphql::daily::Operations operations_;
};

}  // namespace daily::net