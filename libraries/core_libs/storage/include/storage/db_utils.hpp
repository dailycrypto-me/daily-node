#pragma once

#include <rocksdb/slice.h>

namespace daily {
template <typename T>
T FromSlice(rocksdb::Slice const& e) {
  T value;
  memcpy(&value, e.data(), sizeof(T));
  return value;
}
}  // end namespace daily
