// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_TEXT_POSITION_H_
#define ICL_TOOLSETS_SRC_TEXT_POSITION_H_

#include <cstddef>
#include <string_view>
#include <vector>

namespace toolsets {

// Parser byte positions can end inside a code point; count it at its lead byte.
inline size_t Utf16Length(std::string_view text, size_t begin, size_t end) {
  size_t units = 0;
  for (size_t i = begin; i < end; ++i) {
    const auto c = static_cast<unsigned char>(text[i]);
    if ((c & 0xC0) != 0x80) units += c >= 0xF0 ? 2 : 1;
  }
  return units;
}

// One checkpoint per 256 bytes bounds position lookup without a per-byte map.
// The owning document keeps the indexed text immutable.
class Utf16Index {
 public:
  explicit Utf16Index(std::string_view text) {
    prefix_units_.reserve(text.size() / kBlockSize + 1);
    prefix_units_.push_back(0);
    for (size_t end = kBlockSize; end <= text.size(); end += kBlockSize) {
      prefix_units_.push_back(prefix_units_.back() +
                              Utf16Length(text, end - kBlockSize, end));
    }
  }

  size_t Length(std::string_view text, size_t begin, size_t end) const {
    return Prefix(text, end) - Prefix(text, begin);
  }

 private:
  size_t Prefix(std::string_view text, size_t byte) const {
    const size_t block = byte / kBlockSize;
    return prefix_units_[block] + Utf16Length(text, block * kBlockSize, byte);
  }

  static constexpr size_t kBlockSize = 256;
  std::vector<size_t> prefix_units_;
};

}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_TEXT_POSITION_H_
