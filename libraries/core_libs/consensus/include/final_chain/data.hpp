#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/Guards.h>
#include <libdevcore/Log.h>
#include <libdevcore/SHA3.h>

#include "common/encoding_rlp.hpp"
#include "common/types.hpp"
#include "transaction/receipt.hpp"
#include "transaction/transaction.hpp"

namespace daily {
class PbftBlock;
}

namespace daily::final_chain {

/** @addtogroup FinalChain
 * @{
 */

using Nonce = dev::h64;

struct BlockHeaderData {
  h256 parent_hash;
  h256 state_root;
  h256 transactions_root;
  h256 receipts_root;
  LogBloom log_bloom;
  uint64_t gas_used = 0;
  u256 total_reward;
  uint64_t size = 0;

  dev::bytes serializeForDB() const;

  HAS_RLP_FIELDS
};

struct BlockHeader : BlockHeaderData {
  BlockHeader() = default;
  BlockHeader(std::string&& raw_header_data);
  BlockHeader(std::string&& raw_header_data, const PbftBlock& pbft, uint64_t gas_limit);

  void setFromPbft(const PbftBlock& pbft);

  static h256 const& unclesHash();

  static const Nonce& nonce();

  static const u256& difficulty();

  static h256 const& mixHash();

  static std::shared_ptr<BlockHeader> fromRLP(const dev::RLP& rlp);

  void ethereumRlp(dev::RLPStream& encoding) const;
  dev::bytes ethereumRlp() const;

  h256 hash;
  Address author;
  uint64_t gas_limit = 0;
  uint64_t timestamp = 0;
  EthBlockNumber number = 0;
  bytes extra_data;

  HAS_RLP_FIELDS
};

static constexpr auto c_bloomIndexSize = 16;
static constexpr auto c_bloomIndexLevels = 2;

using BlocksBlooms = std::array<LogBloom, c_bloomIndexSize>;

struct NewBlock {
  addr_t author;
  uint64_t timestamp;
  std::vector<h256> dag_blk_hashes;
  h256 hash;
};

struct FinalizationResult : NewBlock {
  std::shared_ptr<BlockHeader const> final_chain_blk;
  SharedTransactions trxs;
  TransactionReceipts trx_receipts;
};

/** @} */

}  // namespace daily::final_chain
