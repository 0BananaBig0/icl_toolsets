// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_UTILITY_TEXT_INPUT_H_
#define ICL_TOOLSETS_SRC_UTILITY_TEXT_INPUT_H_

#include <cstdio>
#include <iostream>
#include <string>

namespace toolsets {

// Iterator-based reads can throw on I/O failures before stream.bad() is
// checked.
inline bool ReadTextStream(std::istream& stream, std::string* text) {
  text->clear();
  char buffer[16384];
  while (stream.read(buffer, sizeof(buffer)) || stream.gcount() > 0) {
    text->append(buffer, static_cast<size_t>(stream.gcount()));
  }
  return !stream.bad() && stream.eof() &&
         !(&stream == &std::cin && std::ferror(stdin));
}

}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_UTILITY_TEXT_INPUT_H_
