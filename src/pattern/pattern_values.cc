// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_values.h"

#include <algorithm>
#include <charconv>
#include <stdexcept>

#include "pdl/pdl_document.h"

namespace pattern {
bool IsReservableResource(const NetlistNode& node) {
  return node.kind == "ScanRegister" || node.kind == "DataRegister" ||
         node.kind == "DataInPort" || node.kind == "DataOutPort" ||
         node.kind == "AddressPort" || node.kind == "ReadEnPort" ||
         node.kind == "WriteEnPort";
}
namespace {
constexpr size_t kMaxBits = 1024 * 1024;
int64_t Index(const std::string& text) {
  int64_t result = 0;
  const auto parsed =
      std::from_chars(text.data(), text.data() + text.size(), result);
  if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
      result < 0) {
    throw std::runtime_error("invalid PDL target bit index");
  }
  return result;
}
size_t Offset(const NetlistNode& node, int64_t index) {
  if (index < std::min(node.left, node.right) ||
      index > std::max(node.left, node.right)) {
    throw std::runtime_error(node.origin + ": PDL target index outside " +
                             node.name);
  }
  return node.left >= node.right ? node.left - index : index - node.left;
}
std::string ResizePdlBits(std::string bits, size_t width) {
  if (bits.empty() || !width || width > kMaxBits) {
    throw std::runtime_error("invalid PDL value width");
  }
  if (bits.size() > width) {
    const size_t extra = bits.size() - width;
    if (bits.substr(0, extra).find('1') != std::string::npos) {
      throw std::runtime_error("PDL value truncates significant bits");
    }
    bits.erase(0, extra);
  }
  bits.insert(0, width - bits.size(), bits[0] == 'x' ? 'x' : '0');
  return bits;
}
}  // namespace
std::string PdlNumber(z3::context& context, const std::string& text,
                      size_t width) {
  if (!width || width > kMaxBits || text.size() > kMaxBits ||
      !pdl::IsNumberLiteral(text)) {
    throw std::runtime_error("invalid or oversized PDL numeric value: " +
                             text.substr(0, 80));
  }
  std::string bits;
  if (text.size() > 2 && text[0] == '0' &&
      (text[1] == 'b' || text[1] == 'B' || text[1] == 'x' || text[1] == 'X')) {
    const size_t count = text[1] == 'b' || text[1] == 'B' ? 1 : 4;
    bits.reserve((text.size() - 2) * count);
    for (size_t i = 2; i < text.size(); ++i) {
      const char c = text[i];
      if (c == 'x' || c == 'X') {
        bits.append(count, 'x');
      } else {
        const unsigned digit = c >= 'a'   ? c - 'a' + 10
                               : c >= 'A' ? c - 'A' + 10
                                          : c - '0';
        for (size_t shift = count; shift > 0; --shift) {
          bits += digit & (1U << (shift - 1)) ? '1' : '0';
        }
      }
    }
  } else {
    const auto first = text.find_first_not_of('0');
    const auto decimal = first == std::string::npos ? "0" : text.substr(first);
    // A conservative decimal-digit bound avoids parsing obviously oversized
    // values.
    if (decimal.size() > width * 30103 / 100000 + 1) {
      throw std::runtime_error("PDL value truncates significant bits");
    }
    context.int_val(decimal.c_str()).as_binary(bits);
    if (bits.empty()) bits = "0";
  }
  return ResizePdlBits(std::move(bits), width);
}
std::string PdlDataValue(z3::context& context, const Netlist& netlist,
                         const PdlTarget& target, const std::string& text) {
  if (pdl::IsNumberLiteral(text)) {
    return PdlNumber(context, text, target.bits.size());
  }
  const auto& node = netlist.nodes.at(target.declaration);
  if (!node.enumeration) {
    throw std::runtime_error(node.origin + ": " + node.name +
                             " has no RefEnum for value " + text);
  }
  const auto& table = netlist.nodes.at(*node.enumeration);
  const auto value = table.constants.find(text);
  if (value == table.constants.end()) {
    throw std::runtime_error(node.origin + ": unknown enum value " + text +
                             " in " + table.name + " for " + node.name);
  }
  return ResizePdlBits(value->second.data, target.bits.size());
}
PdlTarget ResolvePdlTarget(const Netlist& netlist, const std::string& text) {
  const auto open = text.find_first_of("[(");
  const auto name = text.substr(0, open);
  const auto found = netlist.node_ids.find(name);
  if (found == netlist.node_ids.end()) {
    throw std::runtime_error("unknown PDL target " + name);
  }
  const auto& node = netlist.nodes.at(found->second);
  int64_t left = node.left, right = node.right;
  if (open != std::string::npos) {
    if (text.back() != (text[open] == '[' ? ']' : ')')) {
      throw std::runtime_error("unclosed PDL target range");
    }
    const auto range = text.substr(open + 1, text.size() - open - 2);
    const auto colon = range.find(':');
    left = Index(range.substr(0, colon));
    right = colon == std::string::npos ? left : Index(range.substr(colon + 1));
    if (!node.indexed) {
      throw std::runtime_error("PDL scalar target cannot be indexed: " + name);
    }
  }
  Offset(node, left);
  Offset(node, right);
  PdlTarget result;
  result.declaration = found->second;
  const size_t width = left >= right ? left - right + 1 : right - left + 1;
  if (width > kMaxBits) {
    throw std::runtime_error("PDL target width limit exceeded");
  }
  for (int64_t index = left;; index += left <= right ? 1 : -1) {
    BitReference bit{found->second, index};
    size_t traversed = 0;
    while (bit.node != std::numeric_limits<size_t>::max() &&
           netlist.nodes.at(bit.node).model == ModelKind::kAlias) {
      if (++traversed > netlist.nodes.size()) {
        throw std::runtime_error("cyclic PDL alias target");
      }
      const auto& alias = netlist.nodes.at(bit.node);
      if (alias.properties.count("AccessTogether")) {
        result.access_together.insert(bit.node);
      }
      const auto source = alias.inputs.find("Source");
      if (source == alias.inputs.end()) {
        throw std::runtime_error("alias target has no source: " + alias.name);
      }
      auto next = source->second.at(Offset(alias, bit.index));
      next.inverted = next.inverted != bit.inverted;
      bit = next;
    }
    result.bits.push_back(bit);
    if (index == right) break;
  }
  return result;
}
}  // namespace pattern
