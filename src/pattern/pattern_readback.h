// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_

#include <charconv>
#include <string_view>

#include "pattern/pattern_runtime.h"

namespace pattern {
inline constexpr char kModeledReadbackNote[] =
    "modeled register/PIO readback; no live device";
inline constexpr char kStatusClearNote[] = "modeled readback status clear ";
inline bool IsStatusClearNote(const Operation& note) {
  return note.command == "iNote" && note.arguments.size() == 2 &&
         (note.arguments[0] == "-comment" || note.arguments[0] == "-status") &&
         !note.arguments[1].compare(0, sizeof(kStatusClearNote) - 1,
                                    kStatusClearNote);
}
inline std::optional<uint64_t> ReadStatusClearNote(const Operation& note) {
  constexpr size_t kPrefixSize = sizeof(kStatusClearNote) - 1;
  if (!IsStatusClearNote(note) || note.arguments[1].size() <= kPrefixSize ||
      note.arguments[1].size() > kPrefixSize + 20) {
    return std::nullopt;
  }
  const auto text = std::string_view(note.arguments[1]).substr(kPrefixSize);
  uint64_t count = 0;
  const auto parsed =
      std::from_chars(text.data(), text.data() + text.size(), count);
  if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
      text != std::to_string(count)) {
    return std::nullopt;
  }
  return count;
}
inline std::string StatusClearNoteText(const std::string& count) {
  return std::string(kStatusClearNote) + count;
}
// The netlist must outlive this append-only, single-runtime session.
bool CanUsePrimaryCallbackReadback(const Netlist& netlist);
// Replay follows supplied operation order; observed merges must retain it.
Runtime::Readback MakeModeledReadback(const Netlist& netlist);
Runtime::Readback MakeModeledReadback(const Netlist& netlist, bool physical);
Runtime::Readback MakeModeledReadback(
    const Netlist& netlist, bool physical,
    Runtime::StatusReadback* status_readback,
    Runtime::HistoryReadback* history_readback = nullptr);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_READBACK_H_
