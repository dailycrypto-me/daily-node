#include "common/encoding_solidity.hpp"
#include "final_chain/final_chain_impl.hpp"
#include "final_chain/trie_common.hpp"

namespace daily::final_chain {

FinalChainImpl::FinalChainImpl(const std::shared_ptr<DB>& db, const daily::FullNodeConfig& config,
                               const addr_t& node_addr)
    : db_(db),
      kBlockGasLimit(config.genesis.pbft.gas_limit),
      state_api_([this](auto n) { return block_hash(n).value_or(ZeroHash()); },  //
                 config.genesis.state, config.opts_final_chain,
                 {
                     db->stateDbStoragePath().string(),
                 }),
      kLightNode(config.is_light_node),
      kMaxLevelsPerPeriod(config.max_levels_per_period),
      rewards_(
          config.genesis.pbft.committee_size, config.genesis.state.hardforks, db_,
          [this](EthBlockNumber n) { return dpos_eligible_total_vote_count(n); },
          state_api_.get_last_committed_state_descriptor().blk_num),
      block_headers_cache_(config.final_chain_cache_in_blocks, [this](uint64_t blk) { return get_block_header(blk); }),
      block_hashes_cache_(config.final_chain_cache_in_blocks, [this](uint64_t blk) { return get_block_hash(blk); }),
      transactions_cache_(config.final_chain_cache_in_blocks, [this](uint64_t blk) { return get_transactions(blk); }),
      transaction_hashes_cache_(config.final_chain_cache_in_blocks,
                                [this](uint64_t blk) { return get_transaction_hashes(blk); }),
      accounts_cache_(config.final_chain_cache_in_blocks,
                      [this](uint64_t blk, const addr_t& addr) { return state_api_.get_account(blk, addr); }),
      total_vote_count_cache_(config.final_chain_cache_in_blocks,
                              [this](uint64_t blk) { return state_api_.dpos_eligible_total_vote_count(blk); }),
      dpos_vote_count_cache_(
          config.final_chain_cache_in_blocks,
          [this](uint64_t blk, const addr_t& addr) { return state_api_.dpos_eligible_vote_count(blk, addr); }),
      dpos_is_eligible_cache_(
          config.final_chain_cache_in_blocks,
          [this](uint64_t blk, const addr_t& addr) { return state_api_.dpos_is_eligible(blk, addr); }),
      kHardforksConfig(config.genesis.state.hardforks) {
  LOG_OBJECTS_CREATE("EXECUTOR");
  num_executed_dag_blk_ = db_->getStatusField(daily::StatusDbField::ExecutedBlkCount);
  num_executed_trx_ = db_->getStatusField(daily::StatusDbField::ExecutedTrxCount);
  auto state_db_descriptor = state_api_.get_last_committed_state_descriptor();
  auto last_blk_num = db_->lookup_int<EthBlockNumber>(DBMetaKeys::LAST_NUMBER, DB::Columns::final_chain_meta);
  // If we don't have genesis block in db then create and push it
  if (!last_blk_num) [[unlikely]] {
    auto batch = db_->createWriteBatch();
    auto header = append_block(batch, addr_t(), config.genesis.dag_genesis_block.getTimestamp(), kBlockGasLimit,
                               state_db_descriptor.state_root, u256(0));

    block_headers_cache_.append(header->number, header);
    last_block_number_ = header->number;
    db_->commitWriteBatch(batch);
  } else {
    // We need to recover latest changes as there was shutdown inside finalize function
    if (*last_blk_num != state_db_descriptor.blk_num) [[unlikely]] {
      auto batch = db_->createWriteBatch();
      for (auto block_n = *last_blk_num; block_n != state_db_descriptor.blk_num; --block_n) {
        auto raw_period_data = db_->getPeriodDataRaw(block_n);
        assert(raw_period_data.size() > 0);

        const PeriodData period_data(std::move(raw_period_data));
        if (period_data.transactions.size()) {
          num_executed_dag_blk_ -= period_data.dag_blocks.size();
          num_executed_trx_ -= period_data.transactions.size();
        }
        auto period_system_transactions = db_->getPeriodSystemTransactionsHashes(block_n);
        num_executed_trx_ -= period_system_transactions.size();
      }
      db_->insert(batch, DB::Columns::status, StatusDbField::ExecutedBlkCount, num_executed_dag_blk_.load());
      db_->insert(batch, DB::Columns::status, StatusDbField::ExecutedTrxCount, num_executed_trx_.load());
      db_->insert(batch, DB::Columns::final_chain_meta, DBMetaKeys::LAST_NUMBER, state_db_descriptor.blk_num);
      db_->commitWriteBatch(batch);
      last_blk_num = state_db_descriptor.blk_num;
    }
    last_block_number_ = *last_blk_num;

    int64_t start = 0;
    if (*last_blk_num > 5) {
      start = *last_blk_num - 5;
    }
    for (uint64_t num = start; num <= *last_blk_num; ++num) {
      block_headers_cache_.get(num);
    }
  }

  delegation_delay_ = config.genesis.state.dpos.delegation_delay;
  const auto kPruneBlocksToKeep = kDagExpiryLevelLimit + kMaxLevelsPerPeriod + 1;
  if ((config.db_config.prune_state_db || kLightNode) && last_blk_num.has_value() &&
      *last_blk_num > kPruneBlocksToKeep) {
    LOG(log_si_) << "Pruning state db, this might take several minutes";
    prune(*last_blk_num - kPruneBlocksToKeep);
    LOG(log_si_) << "Pruning state db complete";
  }
}

void FinalChainImpl::stop() { executor_thread_.join(); }

std::future<std::shared_ptr<const FinalizationResult>> FinalChainImpl::finalize(
    PeriodData&& new_blk, std::vector<h256>&& finalized_dag_blk_hashes, std::shared_ptr<DagBlock>&& anchor) {
  auto p = std::make_shared<std::promise<std::shared_ptr<const FinalizationResult>>>();
  boost::asio::post(executor_thread_, [this, new_blk = std::move(new_blk),
                                       finalized_dag_blk_hashes = std::move(finalized_dag_blk_hashes),
                                       anchor_block = std::move(anchor), p]() mutable {
    p->set_value(finalize_(std::move(new_blk), std::move(finalized_dag_blk_hashes), std::move(anchor_block)));
    finalized_cv_.notify_one();
  });
  return p->get_future();
}

EthBlockNumber FinalChainImpl::delegation_delay() const { return delegation_delay_; }

SharedTransaction FinalChainImpl::make_bridge_finalization_transaction() {
  const static auto finalize_method = util::EncodingSolidity::packFunctionCall("finalizeEpoch()");
  auto account = get_account(kDailySystemAccount).value_or(state_api::ZeroAccount);

  auto trx = std::make_shared<SystemTransaction>(account.nonce, 0, 0, kBlockGasLimit, finalize_method,
                                                 kHardforksConfig.ficus_hf.bridge_contract_address);
  return trx;
}

bool FinalChainImpl::isNeedToFinalize(EthBlockNumber blk_num) const {
  const static auto get_bridge_root_method = util::EncodingSolidity::packFunctionCall("shouldFinalizeEpoch()");
  return u256(call(state_api::EVMTransaction{dev::ZeroAddress, 1, kHardforksConfig.ficus_hf.bridge_contract_address,
                                             state_api::ZeroAccount.nonce, 0, 10000000, get_bridge_root_method},
                   blk_num)
                  .code_retval)
      .convert_to<bool>();
}

std::vector<SharedTransaction> FinalChainImpl::makeSystemTransactions(PbftPeriod blk_num) {
  std::vector<SharedTransaction> system_transactions;
  // Make system transactions <delegation_delay()> blocks sooner than next pillar block period,
  // e.g.: if pillar block period is 100, this will return true for period 100 - delegation_delay() == 95, 195, 295,
  // etc...
  if (kHardforksConfig.ficus_hf.isPillarBlockPeriod(blk_num + delegation_delay())) {
    if (const auto bridge_contract = get_account(kHardforksConfig.ficus_hf.bridge_contract_address); bridge_contract) {
      if (bridge_contract->code_size && isNeedToFinalize(blk_num - 1)) {
        auto finalize_trx = make_bridge_finalization_transaction();
        system_transactions.push_back(finalize_trx);
      }
    }
  }

  return system_transactions;
}

std::shared_ptr<const FinalizationResult> FinalChainImpl::finalize_(PeriodData&& new_blk,
                                                                    std::vector<h256>&& finalized_dag_blk_hashes,
                                                                    std::shared_ptr<DagBlock>&& anchor) {
  auto batch = db_->createWriteBatch();

  block_applying_emitter_.emit(block_header()->number + 1);

  /*
  // Any dag block producer producing duplicate dag blocks on same level should be slashed


  std::map<std::pair<addr_t, uint64_t>, uint32_t> dag_blocks_per_addr_and_level;
  std::unordered_map<addr_t, uint32_t> duplicate_dag_blocks_count;

  for (const auto& block : new_blk.dag_blocks) {
    dag_blocks_per_addr_and_level[{block.getSender(), block.getLevel()}]++;
  }

  for (const auto& it : dag_blocks_per_addr_and_level) {
    if (it.second > 1) {
      duplicate_dag_blocks_count[it.first.first] += it.second - 1;
    }
  } */

  auto system_transactions = makeSystemTransactions(new_blk.pbft_blk->getPeriod());

  auto all_transactions = new_blk.transactions;
  all_transactions.insert(all_transactions.end(), system_transactions.begin(), system_transactions.end());
  std::vector<state_api::EVMTransaction> evm_trxs;
  append_evm_transactions(evm_trxs, all_transactions);

  const auto& [exec_results] = state_api_.execute_transactions(
      {new_blk.pbft_blk->getBeneficiary(), kBlockGasLimit, new_blk.pbft_blk->getTimestamp(), BlockHeader::difficulty()},
      evm_trxs);
  TransactionReceipts receipts;
  receipts.reserve(exec_results.size());
  std::vector<gas_t> transactions_gas_used;
  transactions_gas_used.reserve(exec_results.size());

  gas_t cumulative_gas_used = 0;
  for (const auto& r : exec_results) {
    LogEntries logs;
    logs.reserve(r.logs.size());
    std::transform(r.logs.cbegin(), r.logs.cend(), std::back_inserter(logs), [](const auto& l) {
      return LogEntry{l.address, l.topics, l.data};
    });
    transactions_gas_used.push_back(r.gas_used);
    receipts.emplace_back(TransactionReceipt{
        r.code_err.empty() && r.consensus_err.empty(),
        r.gas_used,
        cumulative_gas_used += r.gas_used,
        std::move(logs),
        r.new_contract_addr ? std::optional(r.new_contract_addr) : std::nullopt,
    });
  }

  auto rewards_stats = rewards_.processStats(new_blk, transactions_gas_used, batch);
  const auto& [state_root, total_reward] = state_api_.distribute_rewards(rewards_stats);

  auto blk_header =
      append_block(batch, new_blk.pbft_blk->getBeneficiary(), new_blk.pbft_blk->getTimestamp(), kBlockGasLimit,
                   state_root, total_reward, all_transactions, receipts, new_blk.pbft_blk->getExtraDataRlp());

  // Update number of executed DAG blocks and transactions
  auto num_executed_dag_blk = num_executed_dag_blk_ + finalized_dag_blk_hashes.size();
  auto num_executed_trx = num_executed_trx_ + all_transactions.size();
  if (!finalized_dag_blk_hashes.empty()) {
    db_->insert(batch, DB::Columns::status, StatusDbField::ExecutedBlkCount, num_executed_dag_blk);
    db_->insert(batch, DB::Columns::status, StatusDbField::ExecutedTrxCount, num_executed_trx);
    LOG(log_nf_) << "Executed dag blocks #" << num_executed_dag_blk_ - finalized_dag_blk_hashes.size() << "-"
                 << num_executed_dag_blk_ - 1 << " , Transactions count: " << all_transactions.size();
  }

  //// Update DAG lvl mapping
  if (anchor) {
    db_->addProposalPeriodDagLevelsMapToBatch(anchor->getLevel() + kMaxLevelsPerPeriod, new_blk.pbft_blk->getPeriod(),
                                              batch);
  }
  ////

  //// Commit system transactions
  if (!system_transactions.empty()) {
    db_->addPeriodSystemTransactions(batch, system_transactions, new_blk.pbft_blk->getPeriod());
    auto position = new_blk.transactions.size() + 1;
    for (const auto& trx : system_transactions) {
      db_->addSystemTransactionToBatch(batch, trx);
      db_->addTransactionLocationToBatch(batch, trx->getHash(), new_blk.pbft_blk->getPeriod(), position,
                                         true /*system_trx*/);
      position++;
    }
  }
  ////

  auto result = std::make_shared<FinalizationResult>(FinalizationResult{
      {
          new_blk.pbft_blk->getBeneficiary(),
          new_blk.pbft_blk->getTimestamp(),
          std::move(finalized_dag_blk_hashes),
          new_blk.pbft_blk->getBlockHash(),
      },
      blk_header,
      std::move(all_transactions),
      std::move(receipts),
  });

  // Please do not change order of these three lines :)
  db_->commitWriteBatch(batch);
  state_api_.transition_state_commit();
  rewards_.clear(new_blk.pbft_blk->getPeriod());

  num_executed_dag_blk_ = num_executed_dag_blk;
  num_executed_trx_ = num_executed_trx;
  block_headers_cache_.append(blk_header->number, blk_header);
  last_block_number_ = blk_header->number;
  block_finalized_emitter_.emit(result);
  LOG(log_nf_) << " successful finalize block " << result->hash << " with number " << blk_header->number;

  // Creates snapshot if needed
  if (db_->createSnapshot(blk_header->number)) {
    state_api_.create_snapshot(blk_header->number);
  }

  return result;
}

void FinalChainImpl::prune(EthBlockNumber blk_n) {
  LOG(log_nf_) << "Pruning data older than " << blk_n;
  auto last_block_to_keep = get_block_header(blk_n);
  if (last_block_to_keep) {
    auto block_to_keep = last_block_to_keep;
    std::vector<dev::h256> state_root_to_keep;
    while (block_to_keep) {
      state_root_to_keep.push_back(block_to_keep->state_root);
      block_to_keep = get_block_header(block_to_keep->number + 1);
    }
    auto block_to_prune = get_block_header(last_block_to_keep->number - 1);
    while (block_to_prune && block_to_prune->number > 0) {
      db_->remove(DB::Columns::final_chain_blk_by_number, block_to_prune->number);
      db_->remove(DB::Columns::final_chain_blk_hash_by_number, block_to_prune->number);
      db_->remove(DB::Columns::final_chain_blk_number_by_hash, block_to_prune->hash);
      block_to_prune = get_block_header(block_to_prune->number - 1);
    }

    db_->compactColumn(DB::Columns::final_chain_blk_by_number);
    db_->compactColumn(DB::Columns::final_chain_blk_hash_by_number);
    db_->compactColumn(DB::Columns::final_chain_blk_number_by_hash);

    state_api_.prune(state_root_to_keep, last_block_to_keep->number);
  }
}

std::shared_ptr<BlockHeader> FinalChainImpl::append_block(DB::Batch& batch, const addr_t& author, uint64_t timestamp,
                                                          uint64_t gas_limit, const h256& state_root, u256 total_reward,
                                                          const SharedTransactions& transactions,
                                                          const TransactionReceipts& receipts,
                                                          const bytes& extra_data) {
  auto blk_header_ptr = std::make_shared<BlockHeader>();
  auto& blk_header = *blk_header_ptr;
  auto last_block = block_header();
  blk_header.number = last_block ? last_block->number + 1 : 0;
  blk_header.parent_hash = last_block ? last_block->hash : h256();
  blk_header.author = author;
  blk_header.timestamp = timestamp;
  blk_header.state_root = state_root;
  blk_header.gas_used = receipts.empty() ? 0 : receipts.back().cumulative_gas_used;
  blk_header.gas_limit = gas_limit;
  blk_header.total_reward = total_reward;
  blk_header.extra_data = extra_data;
  dev::BytesMap trxs_trie, receipts_trie;
  dev::RLPStream rlp_strm;
  auto trx_idx = 0;
  for (; trx_idx < transactions.size(); ++trx_idx) {
    const auto& trx = transactions[trx_idx];
    auto i_rlp = util::rlp_enc(rlp_strm, trx_idx);
    trxs_trie[i_rlp] = trx->rlp();

    const auto& receipt = receipts[trx_idx];
    receipts_trie[i_rlp] = util::rlp_enc(rlp_strm, receipt);
    db_->insert(batch, DB::Columns::final_chain_receipt_by_trx_hash, trx->getHash(), rlp_strm.out());

    blk_header.log_bloom |= receipt.bloom();
  }
  blk_header.transactions_root = hash256(trxs_trie);
  blk_header.receipts_root = hash256(receipts_trie);
  rlp_strm.clear(), blk_header.ethereum_rlp(rlp_strm);
  blk_header.hash = dev::sha3(rlp_strm.out());
  db_->insert(batch, DB::Columns::final_chain_blk_by_number, blk_header.number, util::rlp_enc(rlp_strm, blk_header));
  auto log_bloom_for_index = blk_header.log_bloom;
  log_bloom_for_index.shiftBloom<3>(sha3(blk_header.author.ref()));
  for (uint64_t level = 0, index = blk_header.number; level < c_bloomIndexLevels; ++level, index /= c_bloomIndexSize) {
    auto chunk_id = block_blooms_chunk_id(level, index / c_bloomIndexSize);
    auto chunk_to_alter = block_blooms(chunk_id);
    chunk_to_alter[index % c_bloomIndexSize] |= log_bloom_for_index;
    db_->insert(batch, DB::Columns::final_chain_log_blooms_index, chunk_id, util::rlp_enc(rlp_strm, chunk_to_alter));
  }
  db_->insert(batch, DB::Columns::final_chain_blk_hash_by_number, blk_header.number, blk_header.hash);
  db_->insert(batch, DB::Columns::final_chain_blk_number_by_hash, blk_header.hash, blk_header.number);
  db_->insert(batch, DB::Columns::final_chain_meta, DBMetaKeys::LAST_NUMBER, blk_header.number);

  return blk_header_ptr;
}

EthBlockNumber FinalChainImpl::last_block_number() const { return last_block_number_; }

std::optional<EthBlockNumber> FinalChainImpl::block_number(const h256& h) const {
  return db_->lookup_int<EthBlockNumber>(h, DB::Columns::final_chain_blk_number_by_hash);
}

std::optional<h256> FinalChainImpl::block_hash(std::optional<EthBlockNumber> n) const {
  return block_hashes_cache_.get(last_if_absent(n));
}

std::shared_ptr<const BlockHeader> FinalChainImpl::block_header(std::optional<EthBlockNumber> n) const {
  if (!n) {
    return block_headers_cache_.last();
  }
  return block_headers_cache_.get(*n);
}

std::optional<TransactionLocation> FinalChainImpl::transaction_location(const h256& trx_hash) const {
  return db_->getTransactionLocation(trx_hash);
}

std::optional<TransactionReceipt> FinalChainImpl::transaction_receipt(const h256& trx_h) const {
  auto raw = db_->lookup(trx_h, DB::Columns::final_chain_receipt_by_trx_hash);
  if (raw.empty()) {
    return {};
  }
  TransactionReceipt ret;
  ret.rlp(dev::RLP(raw));
  return ret;
}

uint64_t FinalChainImpl::transactionCount(std::optional<EthBlockNumber> n) const {
  return db_->getTransactionCount(last_if_absent(n));
}

std::shared_ptr<const TransactionHashes> FinalChainImpl::transaction_hashes(std::optional<EthBlockNumber> n) const {
  return transaction_hashes_cache_.get(last_if_absent(n));
}

const SharedTransactions FinalChainImpl::transactions(std::optional<EthBlockNumber> n) const {
  return transactions_cache_.get(last_if_absent(n));
}

std::vector<EthBlockNumber> FinalChainImpl::withBlockBloom(const LogBloom& b, EthBlockNumber from,
                                                           EthBlockNumber to) const {
  std::vector<EthBlockNumber> ret;
  // start from the top-level
  auto u = int_pow(c_bloomIndexSize, c_bloomIndexLevels);
  // run through each of the top-level blocks
  for (EthBlockNumber index = from / u; index <= (to + u - 1) / u; ++index) {
    dev::operator+=(ret, withBlockBloom(b, from, to, c_bloomIndexLevels - 1, index));
  }
  return ret;
}

std::optional<state_api::Account> FinalChainImpl::get_account(const addr_t& addr,
                                                              std::optional<EthBlockNumber> blk_n) const {
  return accounts_cache_.get(last_if_absent(blk_n), addr);
}

void FinalChainImpl::update_state_config(const state_api::Config& new_config) {
  delegation_delay_ = new_config.dpos.delegation_delay;
  state_api_.update_state_config(new_config);
}

h256 FinalChainImpl::get_account_storage(const addr_t& addr, const u256& key,
                                         std::optional<EthBlockNumber> blk_n) const {
  return state_api_.get_account_storage(last_if_absent(blk_n), addr, key);
}

bytes FinalChainImpl::get_code(const addr_t& addr, std::optional<EthBlockNumber> blk_n) const {
  return state_api_.get_code_by_address(last_if_absent(blk_n), addr);
}

state_api::ExecutionResult FinalChainImpl::call(const state_api::EVMTransaction& trx,
                                                std::optional<EthBlockNumber> blk_n) const {
  auto const blk_header = block_header(last_if_absent(blk_n));
  if (!blk_header) {
    throw std::runtime_error("Future block");
  }
  return state_api_.dry_run_transaction(blk_header->number,
                                        {
                                            blk_header->author,
                                            blk_header->gas_limit,
                                            blk_header->timestamp,
                                            BlockHeader::difficulty(),
                                        },
                                        trx);
}

std::string FinalChainImpl::trace(std::vector<state_api::EVMTransaction> trxs, EthBlockNumber blk_n,
                                  std::optional<state_api::Tracing> params) const {
  const auto blk_header = block_header(last_if_absent(blk_n));
  if (!blk_header) {
    throw std::runtime_error("Future block");
  }
  return dev::asString(state_api_.trace(blk_header->number,
                                        {
                                            blk_header->author,
                                            blk_header->gas_limit,
                                            blk_header->timestamp,
                                            BlockHeader::difficulty(),
                                        },
                                        trxs, params));
}

uint64_t FinalChainImpl::dpos_eligible_total_vote_count(EthBlockNumber blk_num) const {
  return total_vote_count_cache_.get(blk_num);
}

uint64_t FinalChainImpl::dpos_eligible_vote_count(EthBlockNumber blk_num, const addr_t& addr) const {
  return dpos_vote_count_cache_.get(blk_num, addr);
}

bool FinalChainImpl::dpos_is_eligible(EthBlockNumber blk_num, const addr_t& addr) const {
  return dpos_is_eligible_cache_.get(blk_num, addr);
}

vrf_wrapper::vrf_pk_t FinalChainImpl::dpos_get_vrf_key(EthBlockNumber blk_n, const addr_t& addr) const {
  return state_api_.dpos_get_vrf_key(blk_n, addr);
}

std::vector<state_api::ValidatorStake> FinalChainImpl::dpos_validators_total_stakes(EthBlockNumber blk_num) const {
  return state_api_.dpos_validators_total_stakes(blk_num);
}

uint256_t FinalChainImpl::dpos_total_amount_delegated(EthBlockNumber blk_num) const {
  return state_api_.dpos_total_amount_delegated(blk_num);
}

std::vector<state_api::ValidatorVoteCount> FinalChainImpl::dpos_validators_vote_counts(EthBlockNumber blk_num) const {
  return state_api_.dpos_validators_vote_counts(blk_num);
}

void FinalChainImpl::wait_for_finalized() {
  std::unique_lock lck(finalized_mtx_);
  finalized_cv_.wait_for(lck, std::chrono::milliseconds(10));
}

uint64_t FinalChainImpl::dpos_yield(EthBlockNumber blk_num) const { return state_api_.dpos_yield(blk_num); }

u256 FinalChainImpl::dpos_total_supply(EthBlockNumber blk_num) const { return state_api_.dpos_total_supply(blk_num); }

h256 FinalChainImpl::get_bridge_root(EthBlockNumber blk_num) const {
  const static auto get_bridge_root_method = util::EncodingSolidity::packFunctionCall("getBridgeRoot()");
  return h256(call(state_api::EVMTransaction{dev::ZeroAddress, 1, kHardforksConfig.ficus_hf.bridge_contract_address,
                                             state_api::ZeroAccount.nonce, 0, 10000000, get_bridge_root_method},
                   blk_num)
                  .code_retval);
}

h256 FinalChainImpl::get_bridge_epoch(EthBlockNumber blk_num) const {
  const static auto get_bridge_epoch_method = util::EncodingSolidity::packFunctionCall("finalizedEpoch()");
  return h256(call(state_api::EVMTransaction{dev::ZeroAddress, 1, kHardforksConfig.ficus_hf.bridge_contract_address,
                                             state_api::ZeroAccount.nonce, 0, 10000000, get_bridge_epoch_method},
                   blk_num)
                  .code_retval);
}

std::shared_ptr<TransactionHashes> FinalChainImpl::get_transaction_hashes(std::optional<EthBlockNumber> n) const {
  const auto& trxs = db_->getPeriodTransactions(last_if_absent(n));
  auto ret = std::make_shared<TransactionHashes>();
  if (!trxs) {
    return ret;
  }
  ret->reserve(trxs->size());
  std::transform(trxs->cbegin(), trxs->cend(), std::back_inserter(*ret),
                 [](const auto& trx) { return trx->getHash(); });
  return ret;
}

const SharedTransactions FinalChainImpl::get_transactions(std::optional<EthBlockNumber> n) const {
  if (auto trxs = db_->getPeriodTransactions(last_if_absent(n))) {
    return *trxs;
  }
  return {};
}

std::shared_ptr<const BlockHeader> FinalChainImpl::get_block_header(EthBlockNumber n) const {
  if (auto raw = db_->lookup(n, DB::Columns::final_chain_blk_by_number); !raw.empty()) {
    auto ret = std::make_shared<BlockHeader>();
    ret->rlp(dev::RLP(raw));
    return ret;
  }
  return {};
}

std::optional<h256> FinalChainImpl::get_block_hash(EthBlockNumber n) const {
  auto raw = db_->lookup(n, DB::Columns::final_chain_blk_hash_by_number);
  if (raw.empty()) {
    return {};
  }
  return h256(raw, h256::FromBinary);
}

EthBlockNumber FinalChainImpl::last_if_absent(const std::optional<EthBlockNumber>& client_blk_n) const {
  return client_blk_n ? *client_blk_n : last_block_number();
}

state_api::EVMTransaction FinalChainImpl::to_evm_transaction(const SharedTransaction& trx) {
  return state_api::EVMTransaction{
      trx->getSender(), trx->getGasPrice(), trx->getReceiver(), trx->getNonce(),
      trx->getValue(),  trx->getGas(),      trx->getData(),
  };
}

void FinalChainImpl::append_evm_transactions(std::vector<state_api::EVMTransaction>& evm_trxs,
                                             const SharedTransactions& trxs) {
  std::transform(trxs.cbegin(), trxs.cend(), std::back_inserter(evm_trxs),
                 [](const auto& trx) { return to_evm_transaction(trx); });
}

BlocksBlooms FinalChainImpl::block_blooms(const h256& chunk_id) const {
  if (auto raw = db_->lookup(chunk_id, DB::Columns::final_chain_log_blooms_index); !raw.empty()) {
    return dev::RLP(raw).toArray<LogBloom, c_bloomIndexSize>();
  }
  return {};
}

h256 FinalChainImpl::block_blooms_chunk_id(EthBlockNumber level, EthBlockNumber index) {
  return h256(index * 0xff + level);
}

std::vector<EthBlockNumber> FinalChainImpl::withBlockBloom(const LogBloom& b, EthBlockNumber from, EthBlockNumber to,
                                                           EthBlockNumber level, EthBlockNumber index) const {
  std::vector<EthBlockNumber> ret;
  auto uCourse = int_pow(c_bloomIndexSize, level + 1);
  auto uFine = int_pow(c_bloomIndexSize, level);
  auto obegin = index == from / uCourse ? from / uFine % c_bloomIndexSize : 0;
  auto oend = index == to / uCourse ? (to / uFine) % c_bloomIndexSize + 1 : c_bloomIndexSize;
  auto bb = block_blooms(block_blooms_chunk_id(level, index));
  for (auto o = obegin; o < oend; ++o) {
    if (bb[o].contains(b)) {
      // This level has something like what we want.
      if (level > 0) {
        dev::operator+=(ret, withBlockBloom(b, from, to, level - 1, o + index * c_bloomIndexSize));
      } else {
        ret.push_back(o + index * c_bloomIndexSize);
      }
    }
  }
  return ret;
}

}  // namespace daily::final_chain