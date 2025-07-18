#pragma once

#include "pbft/pbft_block.hpp"
#include "vote/pbft_vote.hpp"

namespace daily::network::tarcap {

struct PbftBlocksBundlePacket {
  std::vector<std::shared_ptr<PbftBlock>> pbft_blocks;

  RLP_FIELDS_DEFINE_INPLACE(pbft_blocks)
};

}  // namespace daily::network::tarcap
