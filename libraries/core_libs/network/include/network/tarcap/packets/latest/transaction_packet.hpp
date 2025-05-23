#pragma once

#include "transaction/transaction.hpp"

namespace daily::network::tarcap {

struct TransactionPacket {
  std::vector<std::shared_ptr<Transaction>> transactions;
  std::vector<trx_hash_t> extra_transactions_hashes;

  RLP_FIELDS_DEFINE_INPLACE(transactions, extra_transactions_hashes)
};

}  // namespace daily::network::tarcap
