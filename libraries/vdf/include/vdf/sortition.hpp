#pragma once

#include "common/types.hpp"
#include "common/vrf_wrapper.hpp"
#include "libdevcore/CommonData.h"
#include "vdf/config.hpp"

namespace daily::vdf_sortition {

using namespace vrf_wrapper;

// It includes a vrf for difficulty adjustment
class VdfSortition : public vrf_wrapper::VrfSortitionBase {
 public:
  struct InvalidVdfSortition : std::runtime_error {
    explicit InvalidVdfSortition(std::string const& msg) : runtime_error("Invalid VdfSortition: " + msg) {}
  };

  VdfSortition() = default;
  explicit VdfSortition(SortitionParams const& config, vrf_sk_t const& sk, bytes const& vrf_input, uint64_t vote_count,
                        uint64_t total_vote_count);
  explicit VdfSortition(bytes const& b);
  explicit VdfSortition(Json::Value const& json);

  void computeVdfSolution(const SortitionParams& config, const bytes& msg, const std::atomic_bool& cancelled);

  void verifyVdf(SortitionParams const& config, bytes const& vrf_input, const vrf_pk_t& pk, bytes const& vdf_input,
                 uint64_t vote_count, uint64_t total_vote_count) const;

  bytes rlp() const;
  bool operator==(VdfSortition const& other) const {
    return vrf_wrapper::VrfSortitionBase::operator==(other) && vdf_sol_.first == other.vdf_sol_.first &&
           vdf_sol_.second == other.vdf_sol_.second;
  }
  bool operator!=(VdfSortition const& other) const { return !operator==(other); }

  virtual std::ostream& print(std::ostream& strm) const override {
    VrfSortitionBase::print(strm);
    strm << " Difficulty: " << getDifficulty() << std::endl;
    strm << " Computation Time: " << vdf_computation_time_ << std::endl;
    strm << " Sol1: " << dev::toHex(vdf_sol_.first) << std::endl;
    strm << " Sol2: " << dev::toHex(vdf_sol_.second) << std::endl;
    return strm;
  }
  friend std::ostream& operator<<(std::ostream& strm, VdfSortition const& vdf) { return vdf.print(strm); }

  auto getComputationTime() const { return vdf_computation_time_; }
  uint16_t getDifficulty() const;
  uint16_t calculateDifficulty(SortitionParams const& config) const;
  bool isStale(SortitionParams const& config) const;
  Json::Value getJson() const;

 private:
  inline static dev::bytes N = dev::asBytes(
      "3d1055a514e17cce1290ccb5befb256b00b8aac664e39e754466fcd631004c9e23d16f23"
      "9aee2a207e5173a7ee8f90ee9ab9b6a745d27c6e850e7ca7332388dfef7e5bbe6267d1f7"
      "9f9330e44715b3f2066f903081836c1c83ca29126f8fdc5f5922bf3f9ddb4540171691ac"
      "cc1ef6a34b2a804a18159c89c39b16edee2ede35");
  bool verifyVrf(const vrf_pk_t& pk, const bytes& vrf_input, uint16_t vote_count) const;

  std::pair<bytes, bytes> vdf_sol_;
  unsigned long vdf_computation_time_ = 0;
  uint16_t difficulty_ = 0;
  // Votes are normalized to part per thousand of total votes
  static const uint32_t kVotesProportion = 1000;
  static const uint32_t kThresholdCorrection = 10;
};

}  // namespace daily::vdf_sortition
