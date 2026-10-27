// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_address.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace pattern {
namespace {
constexpr size_t kMissing = std::numeric_limits<size_t>::max();
std::string Number(const Constant& value) {
  if (value.is_string || value.data.empty() ||
      value.data.find_first_not_of("01") != std::string::npos) {
    throw std::runtime_error("address requires a known nonnegative number");
  }
  const auto first = value.data.find('1');
  return first == std::string::npos ? "0" : value.data.substr(first);
}
std::string Add(const std::string& left, const std::string& right) {
  std::string result(std::max(left.size(), right.size()), '0');
  unsigned carry = 0;
  for (size_t i = 0; i < result.size(); ++i) {
    if (i < left.size()) carry += left[left.size() - 1 - i] - '0';
    if (i < right.size()) carry += right[right.size() - 1 - i] - '0';
    result[result.size() - 1 - i] = '0' + (carry & 1);
    carry >>= 1;
  }
  if (carry) result.insert(result.begin(), '1');
  return result;
}
}  // namespace

AddressResolver::AddressResolver(const Netlist& netlist)
    : netlist_(netlist),
      ports_(netlist.scopes.size()),
      domains_(netlist.scopes.size()) {
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    auto& ports = ports_.at(node.scope);
    if (node.kind == "AddressPort") {
      ports.address.push_back(id);
      ports.address_width =
          std::min(kMaxAddressBits + 1, ports.address_width + node.Width());
    }
    if (node.kind == "WriteEnPort") ports.write.push_back(id);
    if (node.kind == "ReadEnPort") ports.read.push_back(id);
  }
}

std::vector<BitReference> AddressResolver::AddressBits(size_t scope) const {
  const auto width = ports_.at(scope).address_width;
  if (width == 0) throw std::runtime_error("missing AddressPort");
  if (width > kMaxAddressBits) {
    throw std::runtime_error("logicalAddress bit limit exceeded");
  }
  std::vector<BitReference> bits;
  bits.reserve(width);
  for (size_t id : ports_.at(scope).address) {
    const auto& node = netlist_.nodes[id];
    for (size_t i = 0; i < node.Width(); ++i) {
      bits.push_back({id, node.left + (node.left <= node.right ? 1 : -1) *
                                          static_cast<int64_t>(i)});
    }
  }
  return bits;
}

bool AddressResolver::ParentAddress(const std::vector<BitReference>& bits,
                                    size_t scope) const {
  if (bits.size() != ports_.at(scope).address_width) return false;
  size_t position = 0;
  for (size_t id : ports_[scope].address) {
    const auto& port = netlist_.nodes[id];
    for (size_t i = 0; i < port.Width(); ++i) {
      const auto& bit = bits[position++];
      const auto index = port.left + (port.left <= port.right ? 1 : -1) *
                                         static_cast<int64_t>(i);
      if (bit.node != id || bit.index != index) return false;
    }
  }
  return true;
}

BitReference AddressResolver::Terminal(BitReference bit, size_t stop_scope,
                                       const std::string& stop_kind) const {
  for (size_t hops = 0; hops <= netlist_.nodes.size(); ++hops) {
    if (bit.node == kMissing || bit.virtual_signal != VirtualSignal::kNone) {
      throw std::runtime_error(
          "disconnected or constant address/control source");
    }
    if (bit.inverted) {
      throw std::runtime_error(
          "inverted address/control source is not a direct connection");
    }
    const auto& node = netlist_.nodes.at(bit.node);
    if (node.inputs.count("Enable")) {
      throw std::runtime_error(
          node.name +
          ": enabled address/control source requires gating constraints");
    }
    // Stop before following a parent's binding: its offset belongs to the
    // inherited domain even when an alias forwards the port.
    if ((node.scope == stop_scope && node.kind == stop_kind) ||
        node.kind == "ScanRegister") {
      return bit;
    }
    if (node.model != ModelKind::kPort && node.model != ModelKind::kAlias) {
      throw std::runtime_error(node.name +
                               ": intervening address/control logic");
    }
    const auto source = node.inputs.find("Source");
    if (source == node.inputs.end()) {
      if (node.scope == 0 &&
          (node.kind == "AddressPort" || node.kind == "WriteEnPort" ||
           node.kind == "ReadEnPort" || node.kind == "DataInPort")) {
        return bit;
      }
      throw std::runtime_error(node.name +
                               ": disconnected address/control source");
    }
    const size_t position =
        static_cast<size_t>(node.left <= node.right ? bit.index - node.left
                                                    : node.left - bit.index);
    bit = source->second.at(position);
  }
  throw std::runtime_error("cyclic address/control connection");
}

const AddressResolver::Domain& AddressResolver::ResolveDomain(size_t scope_id) {
  if (domains_.at(scope_id)) return *domains_[scope_id];
  const auto& scope = netlist_.scopes.at(scope_id);
  if (ports_[scope_id].address.empty()) {
    throw std::runtime_error("missing AddressPort");
  }
  bool explicit_source = false;
  for (size_t id : ports_[scope_id].address) {
    explicit_source |= netlist_.nodes[id].inputs.count("Source") != 0;
  }
  Domain result;
  size_t new_bits = 0;
  if (scope_id != 0 && scope.address_value && !explicit_source) {
    result = ResolveDomain(scope.parent);
  } else {
    auto bits = AddressBits(scope_id);
    if (scope_id != 0) {
      for (auto& bit : bits) {
        const auto& port = netlist_.nodes[bit.node];
        const auto source = port.inputs.find("Source");
        if (source == port.inputs.end()) {
          throw std::runtime_error(port.name + ": disconnected AddressPort");
        }
        const size_t position = static_cast<size_t>(
            port.left <= port.right ? bit.index - port.left
                                    : port.left - bit.index);
        bit =
            Terminal(source->second.at(position), scope.parent, "AddressPort");
      }
    }
    const bool parent_bus = scope_id != 0 && ParentAddress(bits, scope.parent);
    if (parent_bus) {
      result = ResolveDomain(scope.parent);
    } else {
      for (auto& bit : bits) {
        const auto& node = netlist_.nodes[bit.node];
        if (scope_id != 0 && node.scope == scope.parent &&
            node.kind == "AddressPort") {
          throw std::runtime_error(
              "partial or reordered parent logicalAddress mapping");
        }
        if (node.kind == "ScanRegister") {
          if (result.scan_register && *result.scan_register != bit.node) {
            throw std::runtime_error(
                "address/control sources span multiple ScanRegisters");
          }
          result.scan_register = bit.node;
        }
      }
      if (result.scan_register &&
          std::any_of(bits.begin(), bits.end(), [&](const auto& bit) {
            return bit.node != *result.scan_register;
          })) {
        throw std::runtime_error(
            "address/control sources mix primary inputs and ScanRegister");
      }
      new_bits = bits.size();
      result.signals =
          std::make_shared<const std::vector<BitReference>>(std::move(bits));
      result.offset = "0";
    }
  }
  if (scope.address_value) {
    result.offset = Add(result.offset, Number(*scope.address_value));
  }
  if (new_bits > kMaxAddressBits - retained_bits_ ||
      result.offset.size() > kMaxAddressBits - retained_bits_ - new_bits) {
    throw std::runtime_error("resolved address bit limit exceeded");
  }
  retained_bits_ += new_bits + result.offset.size();
  domains_[scope_id] = std::move(result);
  return *domains_[scope_id];
}

BitReference AddressResolver::ResolveEnable(size_t scope_id,
                                            AddressOperation operation) const {
  for (size_t hops = 0; hops < netlist_.scopes.size(); ++hops) {
    const auto& scope = netlist_.scopes.at(scope_id);
    const auto& ports = operation == AddressOperation::kWrite
                            ? ports_[scope_id].write
                            : ports_[scope_id].read;
    const auto kind =
        operation == AddressOperation::kWrite ? "WriteEnPort" : "ReadEnPort";
    if (ports.size() != 1) {
      throw std::runtime_error(std::string("expected one ") + kind);
    }
    const size_t id = ports.front();
    const auto& port = netlist_.nodes[id];
    const auto source = port.inputs.find("Source");
    if (source != port.inputs.end()) {
      auto bit = Terminal(source->second.at(0),
                          scope_id ? scope.parent : kMissing, kind);
      const auto& parent_ports = scope_id
                                     ? (operation == AddressOperation::kWrite
                                            ? ports_[scope.parent].write
                                            : ports_[scope.parent].read)
                                     : ports;
      if (scope_id && parent_ports.size() == 1 &&
          bit.node == parent_ports.front() && !bit.inverted) {
        scope_id = scope.parent;
        continue;
      }
      return bit;
    }
    if (scope_id == 0) return {id, port.left};
    if (!scope.address_value) {
      throw std::runtime_error(port.name + ": disconnected " + kind);
    }
    scope_id = scope.parent;
  }
  throw std::runtime_error("cyclic enable hierarchy");
}

AddressRequirement AddressResolver::Resolve(const std::string& target,
                                            AddressOperation operation) {
  const auto& node = netlist_.NodeAt(target);
  try {
    const auto address = node.constants.find("AddressValue");
    if (node.kind != "DataRegister" || address == node.constants.end()) {
      throw std::runtime_error("not an addressable DataRegister");
    }
    const auto& domain = ResolveDomain(node.scope);
    auto value = Add(domain.offset, Number(address->second));
    if (value.size() > domain.signals->size()) {
      throw std::runtime_error("computed address exceeds logicalAddress width");
    }
    auto enable = ResolveEnable(node.scope, operation);
    if ((domain.scan_register && enable.node != *domain.scan_register) ||
        (!domain.scan_register &&
         netlist_.nodes[enable.node].kind == "ScanRegister")) {
      throw std::runtime_error(
          "address and enable must share primary inputs or one ScanRegister");
    }
    return {domain.signals, {false, false, std::move(value)}, enable};
  } catch (const std::runtime_error& error) {
    throw std::runtime_error(node.origin + ": " + node.name + ": " +
                             error.what());
  }
}
}  // namespace pattern
