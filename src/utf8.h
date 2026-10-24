// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_UTF8_H_
#define ICL_TOOLSETS_SRC_UTF8_H_

#include <cstdint>
#include <string_view>

namespace toolsets {

// Zero means an invalid or incomplete Unicode scalar encoding.
inline size_t Utf8SequenceLength(std::string_view text, size_t begin) {
  if (begin >= text.size()) return 0;
  const auto first = static_cast<unsigned char>(text[begin]);
  if (first < 0x80) return 1;
  if (first < 0xC2 || first > 0xF4) return 0;
  const size_t count = first < 0xE0 ? 2 : first < 0xF0 ? 3 : 4;
  if (text.size() - begin < count) return 0;
  uint32_t codepoint = first & (0x7F >> count);
  for (size_t i = 1; i < count; ++i) {
    const auto next = static_cast<unsigned char>(text[begin + i]);
    if ((next & 0xC0) != 0x80) return 0;
    codepoint = (codepoint << 6) | (next & 0x3F);
  }
  if ((count == 3 && codepoint < 0x800) ||
      (count == 4 && codepoint < 0x10000) || codepoint > 0x10FFFF ||
      (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
    return 0;
  }
  return count;
}

inline bool IsValidUtf8(std::string_view text) {
  for (size_t i = 0; i < text.size();) {
    const auto count = Utf8SequenceLength(text, i);
    if (!count) return false;
    i += count;
  }
  return true;
}

}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_UTF8_H_
