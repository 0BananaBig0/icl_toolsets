// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_BSDL_H_
#define ICL_TOOLSETS_SRC_PATTERN_BSDL_H_

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace pattern {
struct BsdlAttribute {
  std::string target_class;
  std::string expression;
  std::string origin;
};
struct BsdlRegister {
  std::string name;
  std::optional<size_t> width;
};
struct BsdlTap {
  std::string entity, origin;
  // Retain the bounded input so flat runtime metadata can reuse this reader.
  std::string source_path, source;
  size_t instruction_width = 0;
  std::map<std::string, std::string> ports;
  std::map<std::string, std::vector<std::string>> opcodes;
  std::string capture;
  std::optional<std::string> idcode;
  std::optional<std::string> usercode;
  std::optional<size_t> boundary_width;
  std::map<std::string, BsdlRegister> registers;
  // Keys are case-folded VHDL attribute and target identifiers. Retain the
  // original expression for subsequent device-model checks, not execution.
  std::map<std::pair<std::string, std::string>, BsdlAttribute> attributes;
};

// Extracts static TAP metadata, not a full VHDL/BSDL conformance verdict.
// Does not execute code or load external packages. Required unresolved values
// fail explicitly. Returned metadata owns its strings independently of text.
BsdlTap ReadBsdlTap(const std::string& path, const std::string& text,
                    const std::string& entity);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_BSDL_H_
