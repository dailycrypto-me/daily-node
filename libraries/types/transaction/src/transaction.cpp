
#include "transaction/transaction.hpp"

#include <libdevcore/CommonData.h>
#include <libdevcore/CommonJS.h>

#include <algorithm>
#include <string>
#include <utility>

#include "common/constants.hpp"
#include "common/encoding_rlp.hpp"

namespace daily {

uint64_t toChainID(const u256 &val) {
  if (val == 0 || std::numeric_limits<uint64_t>::max() < val) {
    BOOST_THROW_EXCEPTION(Transaction::InvalidTransaction("eip-155 chain id must be in the open interval: (0, 2^64)"));
  }
  return static_cast<uint64_t>(val);
}

TransactionHashes hashes_from_transactions(const SharedTransactions &transactions) {
  TransactionHashes trx_hashes;
  trx_hashes.reserve(transactions.size());
  std::transform(transactions.cbegin(), transactions.cend(), std::back_inserter(trx_hashes),
                 [](const auto &trx) { return trx->getHash(); });
  return trx_hashes;
}

Transaction::Transaction(const trx_nonce_t &nonce, const val_t &value, const val_t &gas_price, gas_t gas, bytes data,
                         const secret_t &sk, const std::optional<addr_t> &receiver, uint64_t chain_id)
    : nonce_(nonce),
      value_(value),
      gas_price_(gas_price),
      gas_(gas),
      data_(std::move(data)),
      receiver_(receiver),
      chain_id_(chain_id),
      vrs_(sign(sk, hash_for_signature())),
      sender_initialized_(true),
      sender_valid_(vrs_.isValid()),
      sender_(sender_valid_ ? toAddress(sk) : dev::ZeroAddress) {
  getSender();
}

Transaction::Transaction(const bytes &_bytes, bool verify_strict) {
  dev::RLP rlp;
  try {
    cached_rlp_ = std::move(_bytes);
    cached_rlp_set_ = true;
    rlp = dev::RLP(cached_rlp_);
  } catch (const dev::RLPException &e) {
    // TODO[1881]: this should be removed when we will add typed transactions support
    std::string error_msg =
        "Can't parse transaction from RLP. Use legacy transactions because typed transactions aren't supported yet.";
    error_msg += "\nException details:\n";
    error_msg += e.what();
    BOOST_THROW_EXCEPTION(dev::RLPException() << dev::errinfo_comment(error_msg));
  }

  fromRLP(rlp, verify_strict);
}

Transaction::Transaction(dev::RLP &&_rlp, bool verify_strict) {
  cached_rlp_ = _rlp.data().toBytes();
  cached_rlp_set_ = true;
  fromRLP(_rlp, verify_strict);
}

void Transaction::fromRLP(const dev::RLP &_rlp, bool verify_strict) {
  u256 v, r, s;
  util::rlp_tuple(util::RLPDecoderRef(_rlp, verify_strict), nonce_, gas_price_, gas_, receiver_, value_, data_, v, r,
                  s);
  // the following piece of code should be equivalent to
  // https://github.com/ethereum/aleth/blob/644d420f8ac0f550a28e9ca65fe1ad1905d0f069/libethcore/TransactionBase.cpp#L58-L78
  // Plus, we don't allow `0` for chain_id. In this code, `chain_id_ == 0` means there is no chain id at all.
  if (!r && !s) {
    chain_id_ = toChainID(v);
  } else {
    if (36 < v) {
      chain_id_ = toChainID((v - 35) / 2);
    } else if (v != 27 && v != 28) {
      BOOST_THROW_EXCEPTION(InvalidFormat(
          "only values 27 and 28 are allowed for non-replay protected transactions for the 'v' signature field"));
    }
    vrs_.v = chain_id_ ? byte{v - (u256{chain_id_} * 2 + 35)} : byte{v - 27};
    vrs_.r = r;
    vrs_.s = s;
  }
}

const trx_hash_t &Transaction::getHash() const {
  std::unique_lock l(hash_mu_);
  if (!hash_initialized_) {
    hash_ = dev::sha3(rlp());
    hash_initialized_ = true;
  }
  return hash_;
}

const addr_t &Transaction::get_sender_() const {
  std::unique_lock l(sender_mu_);
  if (!sender_initialized_) {
    if (auto pubkey = recover(vrs_, hash_for_signature()); pubkey) {
      sender_ = toAddress(pubkey);
      sender_valid_ = true;
    }
    sender_initialized_ = true;
  }
  return sender_;
}

const addr_t &Transaction::getSender() const {
  if (auto const &ret = get_sender_(); sender_valid_) {
    return ret;
  }
  throw InvalidSignature("transaction body: " + toJSON().toStyledString() +
                         "\nOriginal RLP: " + (cached_rlp_set_ ? dev::toJS(cached_rlp_) : "wasn't created from rlp"));
}

void Transaction::streamRLP(dev::RLPStream &s, bool for_signature) const {
  s.appendList(!for_signature || chain_id_ ? 9 : 6);
  s << nonce_ << gas_price_ << gas_;
  if (receiver_) {
    s << *receiver_;
  } else {
    s << 0;
  }
  s << value_ << data_;
  if (!for_signature) {
    s << vrs_.v + uint64_t(chain_id_ ? (chain_id_ * 2 + 35) : 27) << (u256 const &)vrs_.r << (u256 const &)vrs_.s;
  } else if (chain_id_) {
    s << chain_id_ << 0 << 0;
  }
}

const bytes &Transaction::rlp() const {
  std::unique_lock l(cached_rlp_mu_);
  if (!cached_rlp_set_) {
    dev::RLPStream s;
    streamRLP(s, false);
    cached_rlp_ = s.invalidate();
    cached_rlp_set_ = true;
  }
  return cached_rlp_;
}

trx_hash_t Transaction::hash_for_signature() const {
  dev::RLPStream s;
  streamRLP(s, true);
  return dev::sha3(s.out());
}

Json::Value Transaction::toJSON() const {
  Json::Value res(Json::objectValue);
  res["hash"] = dev::toJS(getHash());
  res["from"] = dev::toJS(get_sender_());
  res["nonce"] = dev::toJS(getNonce());
  res["value"] = dev::toJS(getValue());
  res["gasPrice"] = dev::toJS(getGasPrice());
  res["gas"] = dev::toJS(getGas());
  res["sig"] = dev::toJS((sig_t const &)getVRS());
  if (const auto to = getReceiver()) {
    res["to"] = dev::toJS(to.value());
  }
  res["input"] = dev::toJS(getData());
  res["chainId"] = dev::toJS(getChainID());

  const auto &vrs = getVRS();
  res["r"] = dev::toJS(vrs.r);
  res["s"] = dev::toJS(vrs.s);
  res["v"] = dev::toJS(vrs.v);
  return res;
}

void Transaction::rlp(::daily::util::RLPDecoderRef encoding) { fromRLP(encoding.value, false); }

void Transaction::rlp(::daily::util::RLPEncoderRef encoding) const { encoding.appendRaw(rlp()); }

inline uint64_t IntrinsicGas(const std::vector<uint8_t> &data, bool is_contract_creation) {
  uint64_t gas;
  if (is_contract_creation) {
    gas = kTxGasContractCreation;
  } else {
    gas = kTxGas;
  }
  // Bump the required gas by the amount of transactional data
  if (!data.empty()) {
    // Zero and non-zero bytes are priced differently
    uint64_t nz = std::count_if(data.begin(), data.end(), [](uint8_t b) { return b != 0; });

    // Make sure we don't exceed uint64 for all data combinations
    if ((std::numeric_limits<uint64_t>::max() - gas) / kTxDataNonZeroGas < nz) {
      throw std::runtime_error("Out of gas");
    }
    gas += nz * kTxDataNonZeroGas;

    uint64_t z = static_cast<uint64_t>(data.size()) - nz;
    if ((std::numeric_limits<uint64_t>::max() - gas) / kTxDataZeroGas < z) {
      throw std::runtime_error("Out of gas");
    }
    gas += z * kTxDataZeroGas;
  }
  return gas;
}

bool Transaction::intrinsicGasCovered() const {
  try {
    uint64_t gas = IntrinsicGas(data_, !receiver_.has_value());
    return gas <= gas_;
  } catch (const std::runtime_error &) {
    return false;
  }
}

}  // namespace daily
