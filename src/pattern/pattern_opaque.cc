// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_opaque.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>

#include "pattern/pattern_values.h"
#include "pdl/pdl_document.h"

namespace pattern {
namespace {
constexpr size_t kMaxScanBits = 1024 * 1024;
const NetlistNode& OpaqueClient(const Netlist& netlist) {
  const NetlistNode* client = nullptr;
  const std::set<std::string> pins = {
      "ScanInPort",  "ScanOutPort",  "SelectPort", "CaptureEnPort",
      "ShiftEnPort", "UpdateEnPort", "TCKPort",    "ResetPort"};
  if (netlist.scopes.size() != 1 || !netlist.taps.empty()) {
    throw std::runtime_error(
        "opaque scans require a root client without hierarchy or TAPs");
  }
  for (const auto& node : netlist.nodes) {
    if (node.scan_interface) {
      if (client || node.scope != 0 ||
          node.scan_interface->kind != ScanInterfaceKind::kClient ||
          !node.scan_interface->chains.empty()) {
        throw std::runtime_error(
            "opaque scans require one single-chain client");
      }
      client = &node;
    } else if (node.model != ModelKind::kPort || !pins.count(node.kind) ||
               node.scope != 0 || node.Width() != 1 || !node.inputs.empty() ||
               node.reset) {
      throw std::runtime_error(
          "opaque scans require scalar unsourced root pins without modeled "
          "state");
    }
  }
  if (!client || EffectiveScanInterfaces(netlist).size() != 1) {
    throw std::runtime_error("opaque scans require one explicit root client");
  }
  const auto& load = client->scan_interface->default_load;
  if (!load || load->is_string || !load->sized || load->data.empty() ||
      load->data.size() > kMaxScanBits) {
    throw std::runtime_error(
        "opaque scans require a numeric explicitly sized DefaultLoadValue "
        "between 1 and 1048576 bits");
  }
  std::set<std::string> kinds;
  for (const auto& bit : client->scan_interface->ports) {
    if (bit.node >= netlist.nodes.size() || bit.inverted ||
        !kinds.insert(netlist.nodes[bit.node].kind).second) {
      throw std::runtime_error(
          "opaque scan interface pins must be direct and unique");
    }
  }
  if (!kinds.count("ScanInPort") || !kinds.count("ScanOutPort")) {
    throw std::runtime_error(
        "opaque scan interface requires scan input and output");
  }
  return *client;
}
size_t ScanLength(z3::context& context, const std::string& text) {
  const auto bits = PdlNumber(context, text, 21);
  size_t length = 0;
  for (char bit : bits) {
    if (bit == 'x') {
      throw std::runtime_error("scan length must be known");
    }
    length = (length << 1) | (bit == '1');
  }
  if (length == 0 || length > kMaxScanBits) {
    throw std::runtime_error("scan length must be between 1 and 1048576");
  }
  return length;
}
std::string ScanData(z3::context& context, const std::string& text,
                     size_t width) {
  const auto value = pdl::NormalizeScanData(text);
  if (value.size() > 2 && value[0] == '0' &&
      (value[1] == 'b' || value[1] == 'B' || value[1] == 'x' ||
       value[1] == 'X')) {
    const size_t digits = value.size() - 2;
    const size_t bits_per_digit = value[1] == 'b' || value[1] == 'B' ? 1 : 4;
    if (digits > width / bits_per_digit) {
      throw std::runtime_error(
          "scan data exceeds scan length; truncation disallowed");
    }
  }
  try {
    return PdlNumber(context, value, width);
  } catch (const std::runtime_error& error) {
    throw std::runtime_error(std::string("invalid scan data: ") + error.what());
  }
}
}  // namespace

Operation NormalizeScan(z3::context& context, const std::string& target,
                        const Operation& operation) {
  const auto& args = operation.arguments;
  if (args.size() < 2 || args[0] != target) {
    throw std::runtime_error("scan target must be the root client " + target);
  }
  size_t length = 1;
  // Tessent names the sole unnamed chain "default" in exported PDL.
  if (args[1] == "-chain") {
    if (args.size() < 4 || args[2] != "default") {
      throw std::runtime_error("unsupported opaque scan chain selection");
    }
    length = 3;
  }
  const size_t width = ScanLength(context, args[length]);
  std::string input(width, '0'), expected(width, 'x');
  bool stable = false;
  std::set<std::string> seen;
  for (size_t i = length + 1; i < args.size(); ++i) {
    const auto option = args[i];
    if (!seen.insert(option).second) {
      throw std::runtime_error("duplicate opaque scan option");
    }
    if (option == "-stable") {
      stable = true;
    } else if ((option == "-si" || option == "-so") && i + 1 < args.size()) {
      auto bits = ScanData(context, args[++i], width);
      (option == "-si" ? input : expected) = std::move(bits);
    } else {
      throw std::runtime_error("unsupported opaque scan option " + option);
    }
  }
  auto result = operation;
  result.arguments = {target, std::to_string(width), "-si", "0b" + input,
                      "-so",  "0b" + expected};
  if (stable) result.arguments.push_back("-stable");
  return result;
}
Operation NormalizeScan(z3::context& context, const Operation& operation) {
  auto data = operation;
  const bool instruction =
      !data.arguments.empty() && data.arguments[0] == "-ir";
  if (instruction) data.arguments.erase(data.arguments.begin());
  if (data.arguments.size() < 2) {
    throw std::runtime_error("raw scan requires client and length");
  }
  auto result = NormalizeScan(context, data.arguments[0], data);
  if (instruction) result.arguments.insert(result.arguments.begin(), "-ir");
  return result;
}
std::optional<std::vector<Operation>> LowerOpaqueScans(
    const Netlist& netlist, const std::vector<Operation>& operations) {
  if (std::none_of(operations.begin(), operations.end(),
                   [](const auto& op) { return op.command == "iScan"; })) {
    return std::nullopt;
  }
  if (std::any_of(
          netlist.nodes.begin(), netlist.nodes.end(),
          [](const auto& node) { return node.kind == "ScanRegister"; })) {
    return std::nullopt;
  }
  const auto& client = OpaqueClient(netlist);
  z3::context context;
  std::optional<Operation> pending;
  std::vector<Operation> result;
  for (const auto& operation : operations) {
    try {
      if (operation.command == "iScan") {
        // Validate before replacement: even a discarded assignment must be
        // valid.
        pending = NormalizeScan(context, client.name, operation);
      } else if (operation.command == "iApply") {
        ValidatePdlApplyOptions(operation.arguments);
        if (pending) {
          result.push_back(std::move(*pending));
          result.push_back(operation);
        }
        pending.reset();
      } else if (operation.command == "iReset") {
        if (!operation.arguments.empty() &&
            operation.arguments != std::vector<std::string>{"-sync"}) {
          throw std::runtime_error("unsupported opaque iReset option");
        }
        pending.reset();
        result.push_back(operation);
      } else if (operation.command == "iNote") {
        result.push_back(operation);
      } else {
        throw std::runtime_error("unsupported operation with opaque scans: " +
                                 operation.command);
      }
    } catch (const std::exception& error) {
      std::string message = operation.origin + ": " + error.what();
      for (const auto& frame : operation.call_stack) {
        message += "\n  while " + frame;
      }
      throw std::runtime_error(message);
    }
  }
  if (pending) throw std::runtime_error("unapplied opaque scan operations");
  return result;
}
}  // namespace pattern
