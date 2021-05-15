#pragma once

#include <bit>
#include <cstdint>
#include <cstring>

namespace monkey {

inline void PutUint16(uint8_t* dst, uint16_t src) {
  src = (src >> 8) | (src << 8);
  std::memcpy(dst, &src, sizeof(uint16_t));
}

}  // namespace monkey
