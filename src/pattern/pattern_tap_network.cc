// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <limits>
#include <set>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_tap.h"

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
    for (const auto& item : netlist_.node_ids) {
      names_.insert(item.first);
    }
    for (const auto& item : netlist_.port_groups) {
      names_.insert(item.first);
    }
    for (const auto& scope : netlist_.scopes) {
      names_.insert(scope.path);
    }
    const auto& parameters = netlist_.module_parameters.at(netlist_.top_module);
    names_.insert(parameters.begin(), parameters.end());
  }

  void Add(const AccessLink& link, TapRuntimeModel* runtime) {
    const auto tap = static_cast<const TapDatapathNodes&>(*runtime);
    const size_t scope = netlist_.nodes.at(tap.instruction_register).scope;
    for (const auto* role : {"capture", "shift", "update"}) {
      NetlistNode port;
      port.kind = "DataInPort";
      port.scope = scope;
      port.origin = link.origin;
      runtime->controls.emplace(role, AddNode(std::move(port), role));
    }
    const auto endpoint = [&](size_t id) {
      return Endpoint{id, netlist_.nodes.at(id).left};
    };
    const auto root_input =
        endpoint(netlist_.node_ids.at(link.bsdl->ports.at("tap_scan_in")));
    Connect(&netlist_, endpoint(tap.scan_input), root_input, &remaining_);
    Connect(&netlist_,
            endpoint(netlist_.node_ids.at(link.bsdl->ports.at("tap_scan_out"))),
            endpoint(tap.scan_output), &remaining_);
    std::map<Endpoint, std::map<Endpoint, std::set<std::string>>> routes;
    for (const auto& instruction : link.instructions) {
      auto previous = root_input;
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
    std::map<Endpoint, std::set<size_t>> selects;
    std::map<Endpoint, size_t> controls;
    const std::map<std::string, std::string> roles = {
        {"CaptureEnPort", "capture"},
        {"ShiftEnPort", "shift"},
        {"UpdateEnPort", "update"}};
    for (const auto& instruction : link.instructions) {
      for (const auto& selected : instruction.interfaces) {
        const auto& interface =
            *netlist_.NodeAt(selected.instance + "." + selected.name)
                 .scan_interface;
        auto ports = interface.ports;
        if (!interface.chains.empty()) {
          ports.insert(ports.end(), interface.chains[0].ports.begin(),
                       interface.chains[0].ports.end());
        }
        for (const auto& ref : ports) {
          const auto& port = netlist_.nodes.at(ref.node);
          const Endpoint destination{ref.node, ref.index};
          const auto source = port.inputs.find("Source");
          const size_t offset = port.left >= port.right ? port.left - ref.index
                                                        : ref.index - port.left;
          if (source != port.inputs.end() &&
              source->second.at(offset).constant != '?') {
            continue;
          }
          if (port.kind == "SelectPort") {
            selects[destination].insert(
                tap.instruction_selects.at(instruction.name));
          } else if (roles.count(port.kind)) {
            controls[destination] = runtime->controls.at(roles.at(port.kind));
          } else if (port.kind == "TCKPort") {
            controls[destination] =
                netlist_.node_ids.at(link.bsdl->ports.at("tap_scan_clock"));
          }
        }
      }
    }
    for (const auto& [destination, source] : controls) {
      Connect(&netlist_, destination, endpoint(source), &remaining_);
      const auto& kind = netlist_.nodes.at(destination.first).kind;
      if (kind != "TCKPort") {
        rewrites_.push_back({destination, endpoint(source), "To" + kind});
      }
    }
    for (const auto& [destination, sources] : selects) {
      NetlistNode selected;
      selected.kind = "LogicSignal";
      selected.model = ModelKind::kLogic;
      selected.scope = scope;
      selected.origin = link.origin;
      std::vector<size_t> terms;
      for (size_t id : sources) {
        terms.push_back(selected.logic.size());
        selected.logic.push_back(
            {LogicOperation::kSignal, 0, 0, 1, {}, {{id, 0}}});
        selected.inputs["LogicDependencies"].push_back({id, 0});
      }
      while (terms.size() > 1) {
        std::vector<size_t> next;
        for (size_t i = 0; i < terms.size(); i += 2) {
          if (i + 1 == terms.size()) {
            next.push_back(terms[i]);
          } else {
            next.push_back(selected.logic.size());
            selected.logic.push_back(
                {LogicOperation::kOr, terms[i], terms[i + 1], 1, {}, {}});
          }
        }
        terms = std::move(next);
      }
      const auto id = AddNode(std::move(selected), "select");
      Connect(&netlist_, destination, endpoint(id), &remaining_);
      rewrites_.push_back({destination, endpoint(id), "ToSelectPort"});
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
        if (bit == ir_right) {
          break;
        }
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
      selectors_.emplace_back(id, tap.instruction_register);
      Connect(&netlist_, destination, {id, 0}, &remaining_);
    }
  }

  void Finalize() {
    // Append after all TAPs to preserve old node IDs, names and driver
    // ownership.
    for (const auto& rewrite : rewrites_) {
      const auto& destination = netlist_.nodes.at(rewrite.destination.first);
      const size_t offset = destination.left >= destination.right
                                ? destination.left - rewrite.destination.second
                                : rewrite.destination.second - destination.left;
      const auto old = destination.inputs.at("Source").at(offset);
      if (old.node != rewrite.source.first ||
          old.index != rewrite.source.second || old.inverted) {
        throw std::runtime_error(destination.origin +
                                 ": multiple TAP drivers for " +
                                 destination.name);
      }
      const size_t output = Output(rewrite.source.first, rewrite.kind);
      netlist_.nodes.at(rewrite.destination.first)
          .inputs.at("Source")
          .at(offset) = {output, rewrite.source.second};
    }
    for (const auto& [mux, source] : selectors_) {
      const auto& original = netlist_.nodes.at(source);
      const auto& bits = netlist_.nodes.at(mux).inputs.at("SelectedBy");
      if (bits.size() != original.Width()) {
        throw std::runtime_error("changed TAP instruction selector width");
      }
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto index =
            original.left + (original.left <= original.right ? 1 : -1) *
                                static_cast<int64_t>(i);
        if (bits[i].node != source || bits[i].index != index ||
            bits[i].inverted) {
          throw std::runtime_error("changed TAP instruction selector source");
        }
      }
      const size_t output = Output(source, "DataOutPort");
      for (auto& bit : netlist_.nodes.at(mux).inputs.at("SelectedBy")) {
        bit.node = output;
      }
    }
  }

 private:
  size_t Output(size_t source, const std::string& kind) {
    const auto found = outputs_.find(source);
    if (found != outputs_.end()) return found->second;
    const auto& original = netlist_.nodes.at(source);
    Spend(original.Width(), &remaining_);
    NetlistNode port;
    port.kind = kind;
    port.scope = original.scope;
    port.origin = original.origin;
    port.left = original.left;
    port.right = original.right;
    port.indexed = original.indexed;
    auto& bits = port.inputs["Source"];
    bits.reserve(original.Width());
    for (int64_t bit = original.left;;
         bit += original.left <= original.right ? 1 : -1) {
      bits.push_back({source, bit});
      if (bit == original.right) break;
    }
    const size_t id = AddNode(std::move(port), "output");
    outputs_.emplace(source, id);
    return id;
  }

  size_t AddNode(NetlistNode node, const std::string& suffix) {
    const auto& path = netlist_.scopes.at(node.scope).path;
    std::string member;
    do {
      member = "IclPatternControl" + suffix + std::to_string(next_name_++);
      node.name = path.empty() ? member : path + "." + member;
    } while (!names_.insert(node.name).second);
    const size_t id = netlist_.nodes.size();
    if (id >= kMaxObjects) {
      throw std::runtime_error("ICL object expansion limit exceeded");
    }
    netlist_.scopes.at(node.scope).members.emplace(member, id);
    netlist_.node_ids.emplace(node.name, id);
    netlist_.nodes.push_back(std::move(node));
    return id;
  }
  Netlist& netlist_;
  struct Rewrite {
    Endpoint destination, source;
    std::string kind;
  };
  std::vector<Rewrite> rewrites_;
  std::vector<std::pair<size_t, size_t>> selectors_;
  std::map<size_t, size_t> outputs_;
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
  if (links.empty()) {
    for (const auto& [name, model] : result.netlist.taps) {
      result.taps.emplace(name, model);
    }
    return result;
  }
  // Check actual selected bits before any generated TAP wiring is installed.
  for (const auto& link : links) {
    for (const auto& instruction : link.instructions) {
      for (const auto& selected : instruction.interfaces) {
        const auto input = SerialPorts(result.netlist, selected).first;
        const auto& node = result.netlist.nodes[input.first];
        const auto source = node.inputs.find("Source");
        const auto offset = node.left >= node.right ? node.left - input.second
                                                    : input.second - node.left;
        if (source != node.inputs.end() && !source->second.empty() &&
            source->second.at(offset).constant != '?') {
          throw std::runtime_error(selected.origin + ": " + node.name +
                                   " already has an InputPort binding");
        }
      }
    }
  }
  SerialAssembler assembler(&result.netlist);
  for (const auto& link : links) {
    auto nodes = Remap(result.netlist, link.name, models.at(link.name));
    TapRuntimeModel model;
    static_cast<TapDatapathNodes&>(model) = nodes;
    try {
      assembler.Add(link, &model);
    } catch (const std::exception& error) {
      throw std::runtime_error(link.origin + ": AccessLink " + link.name +
                               ": " + error.what());
    }
    model.bsdl = *link.bsdl;
    for (const auto& [role, name] : link.bsdl->ports) {
      model.pins.emplace(role, result.netlist.node_ids.at(name));
    }
    result.netlist.taps.emplace(link.name, std::move(model));
    result.taps.emplace(link.name, std::move(nodes));
  }
  assembler.Finalize();
  CheckCombinationalConnections(result.netlist);
  return result;
}
}  // namespace pattern
