// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <limits>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern_tap.h"

namespace pattern {
namespace {
using Endpoint = std::pair<size_t, int64_t>;
constexpr size_t kMaxModelBytes = 16 * 1024 * 1024;
constexpr size_t kMaxRoutingBits = 4 * 1024 * 1024;
constexpr size_t kMaxObjects = 1000000;

void Spend(size_t amount, size_t* remaining) {
  if (amount > *remaining) {
    throw std::runtime_error("TAP serial connection resource limit exceeded");
  }
  *remaining -= amount;
}

TapDatapathNodes Remap(const Netlist& netlist, const std::string& path,
                       const TapDatapathModel& model) {
  TapDatapathNodes result = model;
  const auto id = [&](size_t original) {
    return netlist.node_ids.at(path + "." +
                               model.netlist.nodes.at(original).name);
  };
  result.instruction_register = id(result.instruction_register);
  result.scan_input = id(result.scan_input);
  result.scan_output = id(result.scan_output);
  result.ir_select = id(result.ir_select);
  for (auto* entries : {&result.instruction_selects, &result.data_registers,
                        &result.scan_returns}) {
    for (auto& entry : *entries) entry.second = id(entry.second);
  }
  return result;
}

std::pair<Endpoint, Endpoint> SerialPorts(const Netlist& netlist,
                                          const AccessLinkInterface& selected) {
  const auto& node = netlist.NodeAt(selected.instance + "." + selected.name);
  const auto& interface = *node.scan_interface;
  if (interface.kind != ScanInterfaceKind::kClient) {
    throw std::runtime_error(
        selected.origin +
        ": TAP serial assembly supports only IJTAG client ScanInterfaces");
  }
  const auto& ports =
      interface.chains.empty() ? interface.ports : interface.chains[0].ports;
  Endpoint input{}, output{};
  for (const auto& ref : ports) {
    const auto& port = netlist.nodes.at(ref.node);
    if (port.kind == "ScanInPort") input = {ref.node, ref.index};
    if (port.kind == "ScanOutPort") output = {ref.node, ref.index};
  }
  // Elaborate has already checked one SI/SO bit per selected chain.
  return {input, output};
}

void Connect(Netlist* netlist, Endpoint destination, Endpoint source,
             size_t* remaining) {
  auto& node = netlist->nodes.at(destination.first);
  auto& bits = node.inputs["Source"];
  if (bits.empty()) {
    Spend(node.Width(), remaining);
    bits.resize(node.Width(), {std::numeric_limits<size_t>::max(), 0, '?'});
  }
  const size_t offset = node.left >= node.right
                            ? node.left - destination.second
                            : destination.second - node.left;
  auto& bit = bits.at(offset);
  if (bit.constant != '?' && (bit.node != source.first ||
                              bit.index != source.second || bit.inverted)) {
    throw std::runtime_error(node.origin + ": multiple TAP drivers for " +
                             node.name);
  }
  bit = {source.first, source.second};
}

class SerialAssembler {
 public:
  explicit SerialAssembler(Netlist* netlist) : netlist_(*netlist) {
    for (const auto& item : netlist_.node_ids) names_.insert(item.first);
    for (const auto& scope : netlist_.scopes) names_.insert(scope.path);
    const auto& parameters = netlist_.module_parameters.at(netlist_.top_module);
    names_.insert(parameters.begin(), parameters.end());
  }

  void Add(const AccessLink& link, const TapDatapathNodes& tap) {
    const auto endpoint = [&](size_t id) {
      return Endpoint{id, netlist_.nodes.at(id).left};
    };
    Connect(&netlist_, endpoint(tap.scan_input),
            endpoint(netlist_.node_ids.at(link.bsdl->ports.at("tap_scan_in"))),
            &remaining_);
    Connect(&netlist_,
            endpoint(netlist_.node_ids.at(link.bsdl->ports.at("tap_scan_out"))),
            endpoint(tap.scan_output), &remaining_);
    std::map<Endpoint, std::map<Endpoint, std::set<std::string>>> routes;
    for (const auto& instruction : link.instructions) {
      auto previous = endpoint(tap.scan_input);
      std::set<Endpoint> seen;
      for (const auto& selected : instruction.interfaces) {
        const auto [input, output] = SerialPorts(netlist_, selected);
        if (!seen.insert(input).second) {
          throw std::runtime_error(selected.origin +
                                   ": scan input repeated in instruction " +
                                   instruction.name);
        }
        auto& codes = routes[input][previous];
        for (const auto& code : link.bsdl->opcodes.at(instruction.name)) {
          if (!codes.count(code)) {
            Spend(code.size(), &remaining_);
            codes.insert(code);
          }
        }
        previous = output;
      }
      if (!instruction.interfaces.empty()) {
        Connect(&netlist_, endpoint(tap.scan_returns.at(instruction.name)),
                previous, &remaining_);
      }
    }
    const auto& ir = netlist_.nodes.at(tap.instruction_register);
    const auto ir_left = ir.left, ir_right = ir.right;
    const size_t ir_width = ir.Width();
    for (const auto& [destination, choices] : routes) {
      if (!driven_.insert(destination).second) {
        throw std::runtime_error(netlist_.nodes[destination.first].origin +
                                 ": scan input shared by multiple AccessLinks");
      }
      Spend(ir_width, &remaining_);
      NetlistNode mux;
      do {
        mux.name = "IclPatternScanRoute" + std::to_string(next_name_++);
      } while (!names_.insert(mux.name).second);
      mux.kind = "ScanMux";
      mux.model = ModelKind::kMux;
      mux.origin = link.origin;
      for (int64_t bit = ir_left;; bit += ir_left <= ir_right ? 1 : -1) {
        mux.inputs["SelectedBy"].push_back({tap.instruction_register, bit});
        if (bit == ir_right) break;
      }
      for (const auto& [source, codes] : choices) {
        MuxChoice choice;
        choice.source.push_back({source.first, source.second});
        for (const auto& code : codes) {
          choice.codes.push_back({false, true, code});
        }
        mux.choices.push_back(std::move(choice));
      }
      const size_t id = netlist_.nodes.size();
      if (id >= kMaxObjects) {
        throw std::runtime_error("ICL object expansion limit exceeded");
      }
      netlist_.node_ids.emplace(mux.name, id);
      netlist_.scopes[0].members.emplace(mux.name, id);
      netlist_.nodes.push_back(std::move(mux));
      Connect(&netlist_, destination, {id, 0}, &remaining_);
    }
  }

 private:
  Netlist& netlist_;
  std::set<std::string> names_;
  std::set<Endpoint> driven_;
  size_t remaining_ = kMaxRoutingBits;
  size_t next_name_ = 0;
};
}  // namespace

TapNetwork ElaborateTapNetwork(Design& design, const std::string& top) {
  const auto& module = design.SelectTop(top);
  const auto links = BindAccessLinks(design, module);
  std::map<std::string, TapDatapathModel> models;
  std::map<std::string, ModuleDefinition> definitions;
  std::map<std::string, std::string> ports;
  static const std::map<std::string, std::string> kKinds = {
      {"tap_scan_in", "ScanInPort"},
      {"tap_scan_out", "ScanOutPort"},
      {"tap_scan_mode", "TMSPort"},
      {"tap_scan_clock", "TCKPort"},
      {"tap_scan_reset", "TRSTPort"}};
  size_t next_model = 0, remaining = kMaxModelBytes;
  for (const auto& link : links) {
    std::string name;
    do {
      name = "IclPatternTapDatapath" + std::to_string(next_model++);
    } while (design.modules().count(name));
    auto model = BuildTapDatapathModel(link, name);
    const auto& definition = model.netlist.scopes[0].definition;
    const auto bytes = definition.document->source().size();
    if (bytes > remaining) {
      throw std::runtime_error(link.origin + ": AccessLink " + link.name +
                               ": aggregate TAP model byte limit exceeded");
    }
    remaining -= bytes;
    definitions.emplace(link.name, definition);
    models.emplace(link.name, std::move(model));
    for (const auto& [role, port] : link.bsdl->ports) {
      const auto inserted = ports.emplace(port, kKinds.at(role));
      if (!inserted.second && inserted.first->second != kKinds.at(role)) {
        throw std::runtime_error(link.origin + ": conflicting TAP roles for " +
                                 port);
      }
    }
  }
  TapNetwork result;
  result.netlist = Elaborate(design, module.name(), definitions, ports);
  if (links.empty()) return result;
  SerialAssembler assembler(&result.netlist);
  for (const auto& link : links) {
    auto nodes = Remap(result.netlist, link.name, models.at(link.name));
    try {
      assembler.Add(link, nodes);
    } catch (const std::exception& error) {
      throw std::runtime_error(link.origin + ": AccessLink " + link.name +
                               ": " + error.what());
    }
    result.taps.emplace(link.name, std::move(nodes));
  }
  CheckCombinationalConnections(result.netlist);
  return result;
}
}  // namespace pattern
