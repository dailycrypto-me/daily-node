#include "graphql/account.hpp"

#include "libdevcore/CommonJS.h"

using namespace std::literals;

namespace graphql::daily {

Account::Account(std::shared_ptr<::daily::final_chain::FinalChain> final_chain, dev::Address address,
                 ::daily::EthBlockNumber blk_n)
    : kAddress(std::move(address)), final_chain_(std::move(final_chain)) {
  account_ = final_chain_->getAccount(kAddress, blk_n);
}

Account::Account(std::shared_ptr<::daily::final_chain::FinalChain> final_chain, dev::Address address)
    : kAddress(std::move(address)), final_chain_(std::move(final_chain)) {
  account_ = final_chain_->getAccount(kAddress);
}

response::Value Account::getAddress() const noexcept { return response::Value(kAddress.toString()); }

response::Value Account::getBalance() const noexcept {
  if (account_) {
    return response::Value(dev::toJS(account_->balance));
  }
  return response::Value(dev::toJS(0));
}

response::Value Account::getTransactionCount() const noexcept {
  if (account_) {
    return response::Value(static_cast<int>(account_->nonce));
  }
  return response::Value(0);
}

response::Value Account::getCode() const noexcept {
  return response::Value(dev::toJS(final_chain_->getCode(kAddress, final_chain_->lastBlockNumber())));
}

response::Value Account::getStorage(response::Value&& slotArg) const {
  return response::Value(dev::toJS(final_chain_->getAccountStorage(kAddress, dev::u256(slotArg.get<std::string>()))));
}

}  // namespace graphql::daily