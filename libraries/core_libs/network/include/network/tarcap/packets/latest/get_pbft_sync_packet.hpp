#pragma once

#include "common/encoding_rlp.hpp"

namespace daily::network::tarcap {

struct GetPbftSyncPacket {
  size_t height_to_sync;

  RLP_FIELDS_DEFINE_INPLACE(height_to_sync)
};

}  // namespace daily::network::tarcap
