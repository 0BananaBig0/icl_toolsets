// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_opaque.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

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

static constexpr char kRawHistoryOmission[] = "modeled raw history omitted ";
bool IsRawHistoryOmissionNote(const Operation& note) {
  return note.command == "iNote" && note.arguments.size() == 2 &&
         (note.arguments[0] == "-comment" || note.arguments[0] == "-status") &&
         !note.arguments[1].compare(0, sizeof(kRawHistoryOmission) - 1,
                                    kRawHistoryOmission);
}
std::string RawHistoryOmissionText(const Operation& scan) {
  std::string result = std::string(kRawHistoryOmission) + scan.command;
  for (const auto& argument : scan.arguments) result += " " + argument;
  return result;
}

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
bool IsModeledChildHistoryNote(const Operation& note) {
  return note.command == "iNote" && note.arguments.size() == 2 &&
         (note.arguments[0] == "-comment" || note.arguments[0] == "-status") &&
         (!note.arguments[1].compare(0, sizeof(kModeledChildHistoryNote) - 1,
                                     kModeledChildHistoryNote) ||
          !note.arguments[1].compare(0, sizeof(kDynamicChildHistoryNote) - 1,
                                     kDynamicChildHistoryNote));
}
std::optional<ModeledChildHistoryNote> ReadModeledChildHistoryNote(
    const Netlist& netlist, const Operation& note) {
  if (!IsModeledChildHistoryNote(note)) return std::nullopt;
  const bool dynamic = !note.arguments[1].compare(
      0, sizeof(kDynamicChildHistoryNote) - 1, kDynamicChildHistoryNote);
  const auto text = std::string_view(note.arguments[1])
                        .substr(dynamic ? sizeof(kDynamicChildHistoryNote) - 1
                                        : sizeof(kModeledChildHistoryNote) - 1);
  const auto first = text.find(' ');
  if (first == std::string_view::npos || first == 0) return std::nullopt;
  const auto second = text.find(' ', first + 1);
  if (second == std::string_view::npos || second == first + 1) {
    return std::nullopt;
  }
  const auto expected = text.substr(second + 1);
  if (expected.size() <= 2 || expected.size() > kMaxScanBits + 2 ||
      expected.substr(0, 2) != "0b" ||
      expected.find_first_not_of("01x", 2) != std::string_view::npos) {
    return std::nullopt;
  }
  const auto root = netlist.node_ids.find(std::string(text.substr(0, first)));
  const auto child = netlist.node_ids.find(
      std::string(text.substr(first + 1, second - first - 1)));
  if (root == netlist.node_ids.end() || child == netlist.node_ids.end()) {
    return std::nullopt;
  }
  for (const auto id : {root->second, child->second}) {
    const auto& node = netlist.nodes[id];
    if (!node.scan_interface ||
        node.scan_interface->kind != ScanInterfaceKind::kClient ||
        !node.scan_interface->chains.empty()) {
      return std::nullopt;
    }
  }
  if (netlist.nodes[root->second].scope ||
      !netlist.nodes[child->second].scope) {
    return std::nullopt;
  }
  return ModeledChildHistoryNote{root->second, child->second,
                                 std::string(expected), dynamic};
}
std::optional<OpaqueChildHistoryNote> ReadOpaqueChildHistoryNote(
    const Netlist& netlist, const Operation& operation) {
  constexpr size_t kPrefixSize = sizeof(kOpaqueChildHistoryNote) - 1;
  if (operation.command != "iNote" || operation.arguments.size() != 2 ||
      (operation.arguments[0] != "-comment" &&
       operation.arguments[0] != "-status") ||
      operation.arguments[1].compare(0, kPrefixSize, kOpaqueChildHistoryNote)) {
    return std::nullopt;
  }
  std::istringstream fields(operation.arguments[1].substr(kPrefixSize));
  std::string root, child, expected, extra;
  if (!(fields >> root >> child >> expected) || fields >> extra ||
      expected.size() <= 2 || expected.size() > kMaxScanBits + 2 ||
      expected.compare(0, 2, "0b") ||
      expected.find_first_not_of("01x", 2) != std::string::npos) {
    return std::nullopt;
  }
  const auto parent = netlist.node_ids.find(root),
             client = netlist.node_ids.find(child);
  if (parent == netlist.node_ids.end() || client == netlist.node_ids.end()) {
    return std::nullopt;
  }
  const auto& root_node = netlist.nodes[parent->second];
  const auto& child_node = netlist.nodes[client->second];
  if (root_node.scope || !child_node.scope || !root_node.scan_interface ||
      !child_node.scan_interface ||
      root_node.scan_interface->kind != ScanInterfaceKind::kClient ||
      child_node.scan_interface->kind != ScanInterfaceKind::kClient ||
      !root_node.scan_interface->chains.empty() ||
      !child_node.scan_interface->chains.empty()) {
    return std::nullopt;
  }
  const auto& load = child_node.scan_interface->default_load;
  if (!load || load->is_string || !load->sized ||
      load->data.size() != expected.size() - 2 ||
      !std::any_of(child_node.scan_interface->ports.begin(),
                   child_node.scan_interface->ports.end(),
                   [&](const auto& bit) {
                     return bit.node < netlist.nodes.size() &&
                            netlist.nodes[bit.node].kind == "ScanOutPort" &&
                            !netlist.nodes[bit.node].inputs.count("Source");
                   })) {
    return std::nullopt;
  }
  return OpaqueChildHistoryNote{parent->second, client->second,
                                std::move(expected)};
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
    if (current.kind == "ScanMux") {
      if (current.Width() != 1 || current.reset || current.inputs.size() != 1 ||
          !current.inputs.count("SelectedBy") ||
          current.inputs.at("SelectedBy").size() != 1 ||
          current.choices.size() != 2) {
        return {};
      }
      std::set<char> codes;
      for (const auto& choice : current.choices) {
        if (choice.source.size() != 1 || choice.codes.size() != 1 ||
            choice.codes[0].data.size() != 1 ||
            (choice.codes[0].data != "0" && choice.codes[0].data != "1") ||
            !codes.insert(choice.codes[0].data[0]).second) {
          return {};
        }
      }
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
  const std::set<std::string> kinds = {
      "ScanInPort",  "ScanOutPort",  "SelectPort", "CaptureEnPort",
      "ShiftEnPort", "UpdateEnPort", "TCKPort",    "ResetPort"};
  struct Child {
    size_t client;
    std::map<std::string, size_t> pins;
    const Constant* load;
  };
  std::vector<Child> children;
  std::map<size_t, size_t> child_scopes, outputs;
  std::set<size_t> parents;
  for (const auto& scope : netlist.scopes) parents.insert(scope.parent);
  for (size_t client : candidates) {
    const auto& node = netlist.nodes[client];
    const auto& interface = *node.scan_interface;
    const auto& load = interface.default_load;
    if (interface.kind != ScanInterfaceKind::kClient ||
        !interface.chains.empty() || !load || load->is_string || !load->sized ||
        load->data.empty() || load->data.size() > kMaxScanBits ||
        parents.count(node.scope) ||
        !child_scopes.emplace(node.scope, children.size()).second) {
      return {};
    }
    std::map<std::string, size_t> pins;
    for (const auto& bit : interface.ports) {
      if (bit.node >= netlist.nodes.size() || bit.inverted) return {};
      const auto& pin = netlist.nodes[bit.node];
      if (pin.scope != node.scope || pin.Width() != 1 ||
          bit.index != pin.left || !kinds.count(pin.kind) ||
          !pins.emplace(pin.kind, bit.node).second) {
        return {};
      }
    }
    if (!pins.count("ScanInPort") || !pins.count("ScanOutPort") ||
        !outputs.emplace(pins.at("ScanOutPort"), children.size()).second) {
      return {};
    }
    children.push_back({client, std::move(pins), &*load});
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
    if (current.model == ModelKind::kRegister &&
        current.kind != "ScanRegister") {
      return {};
    }
    if (child_scopes.count(current.scope) && current.kind != "PortGroup" &&
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
  std::set<size_t> active;
  std::map<Key, char> route;
  std::set<size_t> registers;
  size_t width = 0;
  bool has_mux = false;
  std::vector<std::pair<BitReference, bool>> pending_route{
      {root_pins.at("ScanOutPort"), false}};
  while (!pending_route.empty()) {
    const auto [serial, complete] = pending_route.back();
    pending_route.pop_back();
    if (serial.node == input.node && serial.index == input.index) continue;
    if (serial.node >= netlist.nodes.size()) return {};
    auto& color = route[{serial.node, serial.index}];
    if (complete) {
      color = 2;
      continue;
    }
    if (color == 1) return {};
    if (color == 2) continue;
    color = 1;
    pending_route.emplace_back(serial, true);
    const auto child = outputs.find(serial.node);
    if (child != outputs.end()) {
      if (active.insert(child->second).second) {
        const auto size = children[child->second].load->data.size();
        if (size > kMaxScanBits - width) return {};
        width += size;
      }
      const auto input = children[child->second].pins.at("ScanInPort");
      pending_route.push_back({{input, netlist.nodes[input].left}, false});
      continue;
    }
    const auto& current = netlist.nodes[serial.node];
    if (current.kind == "ScanMux") {
      has_mux = true;
      for (const auto& choice : current.choices) {
        pending_route.emplace_back(choice.source[0], false);
      }
    } else if (current.kind == "ScanRegister") {
      if (serial.index != current.right ||
          !current.inputs.count("ScanInSource") ||
          current.inputs.at("ScanInSource").size() != 1) {
        return {};
      }
      if (registers.insert(serial.node).second) {
        if (current.Width() > kMaxScanBits - width) return {};
        width += current.Width();
      }
      pending_route.emplace_back(current.inputs.at("ScanInSource")[0], false);
    } else {
      const auto next = source(serial);
      if (!next) return {};
      pending_route.emplace_back(*next, false);
    }
  }
  const auto register_count = std::count_if(
      netlist.nodes.begin(), netlist.nodes.end(),
      [](const auto& current) { return current.kind == "ScanRegister"; });
  if (active.size() != children.size() ||
      registers.size() != static_cast<size_t>(register_count)) {
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
    if (!end) return {};
    const bool direct = select != root_pins.end() &&
                        end->node == select->second.node &&
                        end->index == select->second.index && !end->inverted;
    const bool held = has_mux && end->node < netlist.nodes.size() &&
                      netlist.nodes[end->node].kind == "ScanRegister" &&
                      !child_scopes.count(netlist.nodes[end->node].scope);
    if (!direct && !held) return {};
  }
  struct Use {
    Key bit;
    bool forward, terminal;
  };
  std::map<Key, std::vector<Use>> uses;
  for (size_t id = 0; id < netlist.nodes.size(); ++id) {
    const auto& current = netlist.nodes[id];
    for (const auto& [role, bits] : current.inputs) {
      for (size_t offset = 0; offset < bits.size(); ++offset) {
        const auto& bit = bits[offset];
        if (bit.node >= netlist.nodes.size()) continue;
        const auto index = current.left >= current.right
                               ? current.left - static_cast<int64_t>(offset)
                               : current.left + static_cast<int64_t>(offset);
        uses[{bit.node, bit.index}].push_back(
            {{id, index},
             role == "Source" && (current.kind == "ScanInPort" ||
                                  current.kind == "ScanOutPort" ||
                                  current.model == ModelKind::kAlias),
             role == "ScanInSource" && current.kind == "ScanRegister"});
      }
    }
    if (current.kind == "ScanMux") {
      for (const auto& choice : current.choices) {
        const auto& bit = choice.source[0];
        if (bit.node < netlist.nodes.size()) {
          uses[{bit.node, bit.index}].push_back(
              {{id, current.left}, true, false});
        }
      }
    }
    if (current.reset && current.reset->node < netlist.nodes.size()) {
      uses[{current.reset->node, current.reset->index}].push_back(
          {{id, current.left}, false, false});
    }
  }
  std::vector<Key> pending;
  for (const auto& child : children) {
    const auto output = child.pins.at("ScanOutPort");
    pending.emplace_back(output, netlist.nodes[output].left);
  }
  std::set<Key> seen;
  for (size_t i = 0; i < pending.size(); ++i) {
    if (!seen.insert(pending[i]).second) continue;
    const auto found = uses.find(pending[i]);
    if (found == uses.end()) continue;
    for (const auto& use : found->second) {
      if (use.forward) {
        pending.push_back(use.bit);
      } else if (!use.terminal) {
        return {};
      }
    }
  }
  auto model = std::make_shared<OpaqueChildModel>();
  model->netlist = netlist;
  model->source_nodes = netlist.nodes.size();
  model->root = root;
  for (const auto& child : children) {
    const auto& node = netlist.nodes[child.client];
    const auto& pins = child.pins;
    const auto& load = *child.load;
    const auto id = model->netlist.nodes.size();
    model->intervals.push_back({child.client, id, load.data});
    NetlistNode storage;
    storage.name = netlist.scopes.at(node.scope).path + ".__opaque_scan_state";
    while (model->netlist.node_ids.count(storage.name)) storage.name += "_";
    storage.kind = "ScanRegister";
    storage.model = ModelKind::kRegister;
    storage.origin = node.origin;
    storage.scope = node.scope;
    storage.left = static_cast<int64_t>(load.data.size()) - 1;
    storage.indexed = true;
    storage.inputs["ScanInSource"] = {
        {pins.at("ScanInPort"), netlist.nodes[pins.at("ScanInPort")].left}};
    storage.inputs["CaptureSource"].resize(load.data.size());
    auto unknown = load;
    unknown.data.assign(load.data.size(), 'x');
    storage.constants["ResetValue"] = std::move(unknown);
    storage.constants["DefaultLoadValue"] = load;
    if (pins.count("ResetPort")) {
      const auto& reset = netlist.nodes[pins.at("ResetPort")];
      storage.reset =
          BitReference{pins.at("ResetPort"), reset.left, 'x', reset.active_low};
    }
    model->netlist.node_ids.emplace(storage.name, id);
    model->netlist.nodes.push_back(std::move(storage));
    model->netlist.nodes[pins.at("ScanOutPort")].inputs["Source"] = {{id, 0}};
  }
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
      } else if (operation.command == "iNote" ||
                 (operation.command == "iGetStatus" &&
                  operation.arguments.size() == 2 &&
                  operation.arguments[0] == "-clear")) {
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
