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

static Operation NormalizeRequest(z3::context& context,
                                  const std::string& target,
                                  const Operation& operation, bool named) {
  const auto& args = operation.arguments;
  if (args.size() < 2 || args[0] != target) {
    throw std::runtime_error("scan target must be the root client " + target);
  }
  size_t length = 1;
  if (args[1] == "-chain") {
    if (args.size() < 4 || (!named && args[2] != "default")) {
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
  if (named && length == 3) {
    result.arguments.insert(result.arguments.begin() + 1, {"-chain", args[2]});
  }
  if (stable) result.arguments.push_back("-stable");
  return result;
}
Operation NormalizeScan(z3::context& context, const std::string& target,
                        const Operation& operation) {
  return NormalizeRequest(context, target, operation, false);
}
ScanFields ReadScanFields(const Operation& operation) {
  ScanFields fields;
  fields.instruction = operation.arguments.at(0) == "-ir";
  fields.target = fields.instruction ? 1 : 0;
  fields.length = fields.target + 1;
  if (operation.arguments.at(fields.length) == "-chain") {
    fields.chain = operation.arguments.at(fields.length + 1);
    fields.length += 2;
  }
  fields.input = fields.length + 2;
  fields.expected = fields.length + 4;
  return fields;
}
Operation NormalizeScan(z3::context& context, const Operation& operation) {
  auto data = operation;
  const bool instruction =
      !data.arguments.empty() && data.arguments[0] == "-ir";
  if (instruction) data.arguments.erase(data.arguments.begin());
  if (data.arguments.size() < 2) {
    throw std::runtime_error("raw scan requires client and length");
  }
  auto result = NormalizeRequest(context, data.arguments[0], data, true);
  if (instruction) result.arguments.insert(result.arguments.begin(), "-ir");
  return result;
}
OpaqueScanModel DescribeOpaqueScans(const Netlist& netlist) {
  if (netlist.scopes.size() != 1 || !netlist.taps.empty()) {
    throw std::runtime_error(
        "opaque scans require a root client without hierarchy or TAPs");
  }
  const NetlistNode* client = nullptr;
  for (const auto& node : netlist.nodes) {
    if (!node.scan_interface) continue;
    if (client || node.scope != 0 ||
        node.scan_interface->kind != ScanInterfaceKind::kClient) {
      throw std::runtime_error("opaque scans require one root client");
    }
    client = &node;
  }
  if (!client || EffectiveScanInterfaces(netlist).size() != 1) {
    throw std::runtime_error("opaque scans require one explicit root client");
  }
  const auto& interface = *client->scan_interface;
  const bool parallel = !interface.chains.empty();
  const std::set<std::string> pins = {
      "ScanInPort",  "ScanOutPort",  "SelectPort", "CaptureEnPort",
      "ShiftEnPort", "UpdateEnPort", "TCKPort",    "ResetPort"};
  for (const auto& node : netlist.nodes) {
    if (node.scan_interface || node.kind == "PortGroup") continue;
    const bool data = node.kind == "ScanInPort" || node.kind == "ScanOutPort";
    if (node.model != ModelKind::kPort || !pins.count(node.kind) ||
        node.scope != 0 || (node.Width() != 1 && !(parallel && data)) ||
        !node.inputs.empty() || node.reset) {
      throw std::runtime_error(
          "opaque scans require unsourced root pins without modeled state");
    }
  }
  const auto validate_ports = [&](const auto& ports, bool require_data) {
    std::set<std::string> kinds;
    for (const auto& bit : ports) {
      if (bit.node >= netlist.nodes.size() || bit.inverted ||
          !kinds.insert(netlist.nodes[bit.node].kind).second) {
        throw std::runtime_error(
            "opaque scan interface pins must be direct and unique");
      }
    }
    if (require_data &&
        (!kinds.count("ScanInPort") || !kinds.count("ScanOutPort"))) {
      throw std::runtime_error(
          "opaque scan interface requires scan input and output");
    }
  };
  OpaqueScanModel model{client->name, {}};
  const auto add_load = [&](const std::string& name, const auto& load) {
    if (!load || load->is_string || !load->sized || load->data.empty() ||
        load->data.size() > kMaxScanBits) {
      throw std::runtime_error(
          "opaque scans require a numeric explicitly sized DefaultLoadValue "
          "between 1 and 1048576 bits");
    }
    model.defaults.emplace(name, load->data);
  };
  validate_ports(interface.ports, !parallel);
  if (parallel) {
    for (const auto& chain : interface.chains) {
      validate_ports(chain.ports, true);
      add_load(chain.name, chain.default_load);
    }
  } else {
    add_load("", interface.default_load);
  }
  return model;
}
std::shared_ptr<const OpaqueChildModel> ExpandOpaqueChild(
    const Netlist& netlist) {
  if (netlist.scopes.size() <= 1 || !netlist.taps.empty()) return {};
  const std::set<std::string> scan_ports = {
      "ScanInPort",   "ScanOutPort",     "SelectPort",    "CaptureEnPort",
      "ShiftEnPort",  "UpdateEnPort",    "TCKPort",       "ResetPort",
      "ToSelectPort", "ToCaptureEnPort", "ToShiftEnPort", "ToUpdateEnPort",
      "ToTCKPort",    "ToResetPort"};
  for (const auto& current : netlist.nodes) {
    if (current.scan_interface || current.kind == "PortGroup" ||
        current.kind == "ScanRegister") {
      continue;
    }
    if (current.model == ModelKind::kPort && scan_ports.count(current.kind) &&
        current.Width() == 1) {
      continue;
    }
    if (current.model == ModelKind::kAlias && !current.reset &&
        current.inputs.size() == 1 && current.inputs.count("Source") &&
        current.inputs.at("Source").size() == current.Width()) {
      continue;
    }
    return {};
  }
  std::vector<size_t> candidates;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& node = netlist.nodes[id];
    if (!node.scope || !node.scan_interface) continue;
    const auto opaque_output = [&](const auto& ports) {
      return std::any_of(ports.begin(), ports.end(), [&](const auto& bit) {
        return bit.node < netlist.nodes.size() &&
               netlist.nodes[bit.node].kind == "ScanOutPort" &&
               !netlist.nodes[bit.node].inputs.count("Source");
      });
    };
    if (opaque_output(node.scan_interface->ports) ||
        std::any_of(node.scan_interface->chains.begin(),
                    node.scan_interface->chains.end(), [&](const auto& chain) {
                      return opaque_output(chain.ports);
                    })) {
      candidates.push_back(id);
    }
  }
  if (candidates.empty()) return {};
  if (candidates.size() != 1 || !netlist.taps.empty()) {
    return {};
  }
  const auto client = candidates.front();
  const auto& node = netlist.nodes[client];
  const auto& interface = *node.scan_interface;
  if (interface.kind != ScanInterfaceKind::kClient ||
      !interface.chains.empty()) {
    return {};
  }
  const auto& load = interface.default_load;
  if (!load || load->is_string || !load->sized || load->data.empty() ||
      load->data.size() > kMaxScanBits) {
    return {};
  }
  const std::set<std::string> kinds = {
      "ScanInPort",  "ScanOutPort",  "SelectPort", "CaptureEnPort",
      "ShiftEnPort", "UpdateEnPort", "TCKPort",    "ResetPort"};
  std::map<std::string, size_t> pins;
  for (const auto& bit : interface.ports) {
    if (bit.node >= netlist.nodes.size() || bit.inverted) {
      return {};
    }
    const auto& pin = netlist.nodes[bit.node];
    if (pin.scope != node.scope || pin.Width() != 1 || bit.index != pin.left ||
        !kinds.count(pin.kind) || !pins.emplace(pin.kind, bit.node).second) {
      return {};
    }
  }
  if (!pins.count("ScanInPort") || !pins.count("ScanOutPort")) {
    return {};
  }
  for (const auto& scope : netlist.scopes) {
    if (scope.parent == node.scope) return {};
  }
  size_t root = netlist.nodes.size();
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& current = netlist.nodes[id];
    if (current.scan_interface && current.scope == 0) {
      if (root != netlist.nodes.size() ||
          current.scan_interface->kind != ScanInterfaceKind::kClient ||
          !current.scan_interface->chains.empty()) {
        return {};
      }
      root = id;
    }
    if (current.kind == "ScanMux" || (current.model == ModelKind::kRegister &&
                                      current.kind != "ScanRegister")) {
      return {};
    }
    if (current.scope == node.scope && current.kind != "PortGroup" &&
        !current.scan_interface && current.model != ModelKind::kAlias &&
        (current.model != ModelKind::kPort || !kinds.count(current.kind) ||
         current.Width() != 1)) {
      return {};
    }
  }
  if (root == netlist.nodes.size()) return {};
  std::map<std::string, BitReference> root_pins;
  for (const auto& pin : netlist.nodes[root].scan_interface->ports) {
    if (pin.node >= netlist.nodes.size() || pin.inverted ||
        netlist.nodes[pin.node].scope != 0 ||
        !root_pins.emplace(netlist.nodes[pin.node].kind, pin).second) {
      return {};
    }
  }
  if (!root_pins.count("ScanInPort") || !root_pins.count("ScanOutPort")) {
    return {};
  }
  const auto source = [&](BitReference bit) -> std::optional<BitReference> {
    if (bit.node >= netlist.nodes.size()) return {};
    const auto& current = netlist.nodes[bit.node];
    const auto input = current.inputs.find("Source");
    if (input == current.inputs.end()) return {};
    const auto offset = current.left >= current.right
                            ? current.left - bit.index
                            : bit.index - current.left;
    if (offset < 0 || static_cast<size_t>(offset) >= input->second.size()) {
      return {};
    }
    return input->second[static_cast<size_t>(offset)];
  };
  using Key = std::pair<size_t, int64_t>;
  const auto input = root_pins.at("ScanInPort");
  auto serial = root_pins.at("ScanOutPort");
  bool active = false;
  std::set<Key> route;
  std::set<size_t> registers;
  while (serial.node != input.node || serial.index != input.index) {
    if (serial.node >= netlist.nodes.size() ||
        !route.emplace(serial.node, serial.index).second) {
      return {};
    }
    if (serial.node == pins.at("ScanOutPort")) {
      active = true;
      serial = {pins.at("ScanInPort"),
                netlist.nodes[pins.at("ScanInPort")].left};
      continue;
    }
    const auto& current = netlist.nodes[serial.node];
    if (current.kind == "ScanRegister") {
      if (serial.index != current.right ||
          !registers.insert(serial.node).second ||
          !current.inputs.count("ScanInSource") ||
          current.inputs.at("ScanInSource").size() != 1) {
        return {};
      }
      serial = current.inputs.at("ScanInSource")[0];
    } else {
      const auto next = source(serial);
      if (!next) return {};
      serial = *next;
    }
  }
  const auto register_count = std::count_if(
      netlist.nodes.begin(), netlist.nodes.end(),
      [](const auto& current) { return current.kind == "ScanRegister"; });
  if (!active || registers.size() != static_cast<size_t>(register_count)) {
    return {};
  }

  std::map<Key, BitReference> terminals;
  const auto terminal = [&](BitReference bit) -> std::optional<BitReference> {
    std::vector<std::pair<Key, bool>> trail;
    std::set<Key> visited;
    bool inverted = false;
    BitReference result;
    while (true) {
      inverted = inverted != bit.inverted;
      bit.inverted = false;
      if (bit.node >= netlist.nodes.size()) {
        result = bit;
        result.inverted = inverted;
        break;
      }
      const Key key{bit.node, bit.index};
      const auto cached = terminals.find(key);
      if (cached != terminals.end()) {
        result = cached->second;
        result.inverted = result.inverted != inverted;
        break;
      }
      if (!visited.insert(key).second) return {};
      const auto next = source(bit);
      if (!next) {
        result = bit;
        result.inverted = inverted;
        break;
      }
      trail.emplace_back(key, inverted);
      bit = *next;
    }
    for (const auto& [key, prefix] : trail) {
      auto value = result;
      value.inverted = value.inverted != prefix;
      terminals.emplace(key, value);
    }
    return result;
  };
  for (const auto& current : netlist.nodes) {
    if (current.scope == 0 || current.kind != "SelectPort") continue;
    const auto end =
        terminal({netlist.node_ids.at(current.name), current.left});
    const auto select = root_pins.find("SelectPort");
    if (!end || select == root_pins.end() || end->node != select->second.node ||
        end->index != select->second.index || end->inverted) {
      return {};
    }
  }
  struct Use {
    size_t node;
    bool forward, terminal;
  };
  std::vector<std::vector<Use>> uses(netlist.nodes.size());
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& current = netlist.nodes[id];
    for (const auto& [role, bits] : current.inputs) {
      for (const auto& bit : bits) {
        if (bit.node >= uses.size()) continue;
        uses[bit.node].push_back(
            {id,
             role == "Source" && (current.kind == "ScanInPort" ||
                                  current.kind == "ScanOutPort" ||
                                  current.model == ModelKind::kAlias),
             role == "ScanInSource" && current.kind == "ScanRegister"});
      }
    }
    if (current.reset && current.reset->node < uses.size()) {
      uses[current.reset->node].push_back({id, false, false});
    }
  }
  std::vector<size_t> pending{pins.at("ScanOutPort")};
  std::vector<bool> seen(netlist.nodes.size(), false);
  for (size_t i = 0; i < pending.size(); ++i) {
    if (seen[pending[i]]) continue;
    seen[pending[i]] = true;
    for (const auto& use : uses[pending[i]]) {
      if (use.forward) {
        pending.push_back(use.node);
      } else if (!use.terminal) {
        return {};
      }
    }
  }
  auto model = std::make_shared<OpaqueChildModel>();
  model->netlist = netlist;
  model->source_nodes = netlist.nodes.size();
  model->client = client;
  model->root = root;
  model->storage = netlist.nodes.size();
  model->default_load = load->data;
  NetlistNode storage;
  storage.name = netlist.scopes.at(node.scope).path + ".__opaque_scan_state";
  while (model->netlist.node_ids.count(storage.name)) storage.name += "_";
  storage.kind = "ScanRegister";
  storage.model = ModelKind::kRegister;
  storage.origin = node.origin;
  storage.scope = node.scope;
  storage.left = static_cast<int64_t>(load->data.size()) - 1;
  storage.indexed = true;
  storage.inputs["ScanInSource"] = {
      {pins.at("ScanInPort"), netlist.nodes[pins.at("ScanInPort")].left}};
  storage.inputs["CaptureSource"].resize(load->data.size());
  auto unknown = *load;
  unknown.data.assign(load->data.size(), 'x');
  storage.constants["ResetValue"] = std::move(unknown);
  storage.constants["DefaultLoadValue"] = *load;
  if (pins.count("ResetPort")) {
    const auto& reset = netlist.nodes[pins.at("ResetPort")];
    storage.reset =
        BitReference{pins.at("ResetPort"), reset.left, 'x', reset.active_low};
  }
  model->netlist.node_ids.emplace(storage.name, model->storage);
  model->netlist.nodes.push_back(std::move(storage));
  model->netlist.nodes[pins.at("ScanOutPort")].inputs["Source"] = {
      {model->storage, 0}};
  return model;
}
Operation NormalizeOpaqueScan(z3::context& context,
                              const OpaqueScanModel& model,
                              const Operation& operation) {
  const bool scalar = model.defaults.count("");
  auto scan = scalar ? NormalizeScan(context, model.client, operation)
                     : NormalizeScan(context, operation);
  const auto fields = ReadScanFields(scan);
  if (fields.instruction || scan.arguments[fields.target] != model.client ||
      !model.defaults.count(fields.chain)) {
    throw std::runtime_error("opaque scan requires a valid client chain");
  }
  return scan;
}
void OpaqueScanQueue::Add(z3::context& context, const Operation& operation) {
  auto scan = NormalizeOpaqueScan(context, model_, operation);
  const auto fields = ReadScanFields(scan);
  const auto found = pending_.find(fields.chain);
  const size_t previous =
      found == pending_.end()
          ? 0
          : found->second.arguments[ReadScanFields(found->second).input]
                    .size() -
                2;
  const size_t bits =
      pending_bits_ - previous + scan.arguments[fields.input].size() - 2;
  if (bits > kMaxScanBits) {
    throw std::runtime_error("opaque scan queued payload resource limit");
  }
  pending_.insert_or_assign(fields.chain, std::move(scan));
  pending_bits_ = bits;
}
void OpaqueScanQueue::Reset(const Operation& operation) {
  if (!operation.arguments.empty() &&
      operation.arguments != std::vector<std::string>{"-sync"}) {
    throw std::runtime_error("unsupported opaque iReset option");
  }
  pending_.clear();
  pending_bits_ = 0;
  stable_.clear();
  defaults_available_ = true;
  unknown_lengths_.clear();
}
std::vector<Operation> OpaqueScanQueue::Apply(const Operation& operation) {
  ValidatePdlApplyOptions(operation.arguments);
  if (pending_.empty()) return {};
  return Complete(operation);
}
std::vector<Operation> OpaqueScanQueue::Complete(const Operation& operation) {
  ValidatePdlApplyOptions(operation.arguments);
  const bool parallel = !model_.defaults.count("");
  size_t bits = pending_bits_;
  for (const auto& [name, load] : model_.defaults) {
    if (pending_.count(name)) continue;
    const auto known = stable_.find(name);
    if (known == stable_.end() &&
        (!defaults_available_ || unknown_lengths_.count(name))) {
      throw std::runtime_error(
          parallel ? "opaque parallel scan chain length is unknown"
                   : "opaque scan chain length is unknown");
    }
    const size_t width =
        known == stable_.end()
            ? load.size()
            : known->second.arguments[ReadScanFields(known->second).input]
                      .size() -
                  2;
    if (width > kMaxScanBits - bits) {
      throw std::runtime_error("opaque scan queued payload resource limit");
    }
    bits += width;
  }
  auto executed = pending_;
  for (const auto& [name, load] : model_.defaults) {
    if (executed.count(name)) continue;
    const auto known = stable_.find(name);
    auto scan = known == stable_.end() ? operation : known->second;
    if (known == stable_.end()) {
      scan.command = "iScan";
      scan.arguments = {model_.client};
      if (parallel) {
        scan.arguments.insert(scan.arguments.end(), {"-chain", name});
      }
      scan.arguments.insert(
          scan.arguments.end(),
          {std::to_string(load.size()), "-si", "0b" + load, "-so",
           "0b" + std::string(load.size(), 'x'), "-stable"});
    } else {
      const auto fields = ReadScanFields(scan);
      scan.arguments[fields.expected] =
          "0b" + std::string(scan.arguments[fields.input].size() - 2, 'x');
    }
    executed.emplace(name, std::move(scan));
  }
  std::vector<Operation> result;
  result.reserve(executed.size() + 1);
  for (auto& [name, scan] : executed) result.push_back(std::move(scan));
  result.push_back(operation);
  for (const auto& [name, scan] : pending_) {
    if (scan.arguments.back() == "-stable") {
      stable_.insert_or_assign(name, scan);
      unknown_lengths_.erase(name);
    } else {
      stable_.erase(name);
      unknown_lengths_.insert(name);
    }
  }
  pending_.clear();
  pending_bits_ = 0;
  return result;
}
std::optional<std::vector<Operation>> LowerOpaqueScans(
    const Netlist& netlist, const std::vector<Operation>& operations) {
  if (netlist.scopes.size() > 1) return std::nullopt;
  if (std::none_of(operations.begin(), operations.end(),
                   [](const auto& op) { return op.command == "iScan"; })) {
    return std::nullopt;
  }
  if (std::any_of(
          netlist.nodes.begin(), netlist.nodes.end(),
          [](const auto& node) { return node.kind == "ScanRegister"; })) {
    return std::nullopt;
  }
  OpaqueScanQueue queue(DescribeOpaqueScans(netlist));
  z3::context context;
  std::vector<Operation> result;
  for (const auto& operation : operations) {
    try {
      if (operation.command == "iScan") {
        // Validate before replacement: even a discarded assignment must be
        // valid.
        queue.Add(context, operation);
      } else if (operation.command == "iApply") {
        auto executed = queue.Apply(operation);
        result.insert(result.end(), executed.begin(), executed.end());
      } else if (operation.command == "iReset") {
        queue.Reset(operation);
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
  if (!queue.Pending().empty()) {
    throw std::runtime_error("unapplied opaque scan operations");
  }
  return result;
}
}  // namespace pattern
