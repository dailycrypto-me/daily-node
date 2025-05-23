#include "graphql/transaction.hpp"

#include <optional>

#include "graphql/account.hpp"
#include "graphql/log.hpp"
#include "libdevcore/CommonJS.h"

using namespace std::literals;

namespace graphql::daily {

Transaction::Transaction(std::shared_ptr<::daily::final_chain::FinalChain> final_chain,
                         std::shared_ptr<::daily::TransactionManager> trx_manager,
                         std::function<std::shared_ptr<object::Block>(::daily::EthBlockNumber)> get_block_by_num,
                         std::shared_ptr<::daily::Transaction> transaction) noexcept
    : final_chain_(std::move(final_chain)),
      trx_manager_(std::move(trx_manager)),
      get_block_by_num_(std::move(get_block_by_num)),
      transaction_(std::move(transaction)),
      location_(*final_chain_->transactionLocation(transaction_->getHash())) {}

response::Value Transaction::getHash() const noexcept { return response::Value(transaction_->getHash().toString()); }

response::Value Transaction::getNonce() const noexcept { return response::Value(transaction_->getNonce().str()); }

std::optional<int> Transaction::getIndex() const noexcept { return {location_.position}; }

std::shared_ptr<object::Account> Transaction::getFrom(std::optional<response::Value>&&) const {
  return std::make_shared<object::Account>(
      std::make_shared<Account>(final_chain_, transaction_->getSender(), location_.period));
}

std::shared_ptr<object::Account> Transaction::getTo(std::optional<response::Value>&&) const {
  if (!transaction_->getReceiver()) return nullptr;
  return std::make_shared<object::Account>(
      std::make_shared<Account>(final_chain_, *transaction_->getReceiver(), location_.period));
}

response::Value Transaction::getValue() const noexcept { return response::Value(transaction_->getValue().str()); }

response::Value Transaction::getGasPrice() const noexcept { return response::Value(transaction_->getGasPrice().str()); }

response::Value Transaction::getGas() const noexcept {
  return response::Value(static_cast<int>(transaction_->getGas()));
}

response::Value Transaction::getInputData() const noexcept {
  return response::Value(dev::toJS(transaction_->getData()));
}

std::shared_ptr<object::Block> Transaction::getBlock() const { return get_block_by_num_(location_.period); }

std::optional<response::Value> Transaction::getStatus() const noexcept {
  if (!receipt_) {
    receipt_ = final_chain_->transactionReceipt(location_.period, location_.position, transaction_->getHash());
    if (!receipt_) return std::nullopt;
  }
  return response::Value(static_cast<int>(receipt_->status_code));
}

std::optional<response::Value> Transaction::getGasUsed() const noexcept {
  if (!receipt_) {
    receipt_ = final_chain_->transactionReceipt(location_.period, location_.position, transaction_->getHash());
    if (!receipt_) return std::nullopt;
  }
  return response::Value(static_cast<int>(receipt_->gas_used));
}

std::optional<response::Value> Transaction::getCumulativeGasUsed() const noexcept {
  if (!receipt_) {
    receipt_ = final_chain_->transactionReceipt(location_.period, location_.position, transaction_->getHash());
    if (!receipt_) return std::nullopt;
  }
  return response::Value(static_cast<int>(receipt_->cumulative_gas_used));
}

std::shared_ptr<object::Account> Transaction::getCreatedContract(std::optional<response::Value>&&) const noexcept {
  if (!receipt_) {
    receipt_ = final_chain_->transactionReceipt(location_.period, location_.position, transaction_->getHash());
    if (!receipt_) return nullptr;
  }
  if (!receipt_->new_contract_address) return nullptr;
  return std::make_shared<object::Account>(std::make_shared<Account>(final_chain_, *receipt_->new_contract_address));
}

std::optional<std::vector<std::shared_ptr<object::Log>>> Transaction::getLogs() const noexcept {
  std::vector<std::shared_ptr<object::Log>> logs;
  if (!receipt_) {
    receipt_ = final_chain_->transactionReceipt(location_.period, location_.position, transaction_->getHash());
    if (!receipt_) return std::nullopt;
  }

  for (int i = 0; i < static_cast<int>(receipt_->logs.size()); ++i) {
    logs.push_back(std::make_shared<object::Log>(
        std::make_shared<Log>(final_chain_, trx_manager_, shared_from_this(), receipt_->logs[i], i)));
  }

  return logs;
}

response::Value Transaction::getR() const noexcept { return response::Value(dev::toJS(transaction_->getVRS().r)); }

response::Value Transaction::getS() const noexcept { return response::Value(dev::toJS(transaction_->getVRS().s)); }

response::Value Transaction::getV() const noexcept { return response::Value(dev::toJS(transaction_->getVRS().v)); }

}  // namespace graphql::daily