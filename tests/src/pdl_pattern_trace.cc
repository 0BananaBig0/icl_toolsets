// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
// Static operation comparison only; this never executes input Tcl.
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "pattern/pattern_tap.h"
#include "pattern/pattern_values.h"
#include "pdl/pdl_document.h"
#include "utility/json.h"
#include "utility/text_input.h"

namespace {
using icl::Json;

std::string ReadFile(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  std::string text;
  if (!input || !toolsets::ReadTextStream(input, &text))
    throw std::runtime_error("cannot read input: " + path);
  return text;
}

using BitValues = std::map<std::pair<std::string, int64_t>, char>;
Json BitRows(const BitValues& values) {
  Json::Elements rows;
  for (const auto& [key, value] : values)
    rows.push_back(Json::Array({Json::String(key.first), Json::Int(key.second),
                                Json::String(std::string(1, value))}));
  return Json::Array(std::move(rows));
}

Json ApplyGroups(const Json& trace, const pattern::Netlist& netlist,
                 const std::string& top) {
  const auto binding = trace.Find("binding")->string_value();
  if (!binding.empty() && binding != top)
    throw std::runtime_error("trace binding differs from comparison top");
  z3::context context;
  BitValues writes, reads, states;
  Json::Elements events;
  size_t expanded = 0;
  const auto flush_states = [&] {
    if (!states.empty()) {
      events.push_back(Json::Object({{"iState", BitRows(states)}}));
      states.clear();
    }
  };
  for (const auto& operation : trace.Find("operations")->elements()) {
    const auto& words = operation.elements();
    const auto name = words.at(0).string_value();
    if (name != "iState") flush_states();
    if (name == "iWrite" || name == "iRead" || name == "iState") {
      if (words.size() < 2 || words.size() > (name == "iState" ? 4U : 3U) ||
          (words.size() == 2 && name != "iRead") ||
          (words.size() == 4 && words[3].string_value() != "-LastWrittenValue"))
        throw std::runtime_error("unsupported apply-group assignment options");
      if (name == "iState" && (!writes.empty() || !reads.empty()))
        throw std::runtime_error("state assumption with pending actions");
      const auto target =
          pattern::ResolvePdlTarget(netlist, words[1].string_value());
      if (!target.access_together.empty())
        throw std::runtime_error(
            "AccessTogether obligations need explicit comparison support");
      expanded += target.bits.size();
      if (expanded > 256 * 1024)
        throw std::runtime_error("apply-group bit expansion limit exceeded");
      const auto bits = words.size() == 2
                            ? std::string(target.bits.size(), 'x')
                            : pattern::PdlDataValue(context, netlist, target,
                                                    words[2].string_value());
      auto& destination = name == "iWrite"  ? writes
                          : name == "iRead" ? reads
                                            : states;
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto& bit = target.bits[i];
        if (bit.node == std::numeric_limits<size_t>::max())
          throw std::runtime_error("constant target in apply-group comparison");
        const auto key =
            std::make_pair(netlist.nodes.at(bit.node).name, bit.index);
        char value = bits[i];
        if (bit.inverted && value != 'x') value = value == '0' ? '1' : '0';
        // X assumptions retain prior known fill; preserve overlapping history,
        // including repeated physical bits within one alias.
        if (name == "iState" && states.count(key)) flush_states();
        // X writes retain earlier controlling assignments in the same queue.
        if (name != "iWrite" || value != 'x' || !destination.count(key))
          destination.insert_or_assign(key, value);
      }
    } else if (name == "iApply") {
      events.push_back(Json::Object({{"writes", BitRows(writes)},
                                     {"reads", BitRows(reads)},
                                     {"apply", operation}}));
      writes.clear();
      reads.clear();
    } else {
      if (!writes.empty() || !reads.empty())
        throw std::runtime_error("unapplied actions before " + name);
      events.push_back(operation);
    }
  }
  flush_states();
  if (!writes.empty() || !reads.empty())
    throw std::runtime_error("unapplied actions at end of trace");
  return Json::Object({{"binding", Json::String(top)},
                       {"operations", Json::Array(std::move(events))}});
}

void RequireStatic(const pdl::Command& command) {
  for (const auto& word : command.words) {
    if (!word.literal || word.expanded || !word.variables.empty() ||
        (word.command_substitution && !word.vector_substitution)) {
      throw std::runtime_error("dynamic word in static pattern trace");
    }
  }
}

Json Trace(const pdl::Document& document, const std::string& entry) {
  if (!document.ok()) throw std::runtime_error(document.error_message());
  const std::vector<pdl::Command>* commands = &document.commands();
  bool found = false;
  std::string binding;
  for (const auto& command : document.commands()) {
    if (command.words.empty()) continue;
    RequireStatic(command);
    const auto& words = command.words;
    const auto& name = words[0].value;
    if (name == "iProcsForModule") {
      if (words.size() != 2 || !binding.empty()) {
        throw std::runtime_error("ambiguous module binding");
      }
      binding = words[1].value;
    } else if (name == "iProc") {
      if (entry.empty()) throw std::runtime_error("unflattened iProc");
      if (words.size() != 4) throw std::runtime_error("invalid iProc");
      if (words[1].value != entry) continue;
      bool valid = false;
      const auto parameters = pdl::SplitList(words[2].value, &valid);
      if (found || !valid || !parameters.empty() || !words[3].body) {
        throw std::runtime_error("ambiguous or parameterized entry");
      }
      found = true;
      commands = &words[3].script;
    } else if (!entry.empty() && name != "iPDLLevel" && name != "iNote") {
      throw std::runtime_error("executable main alongside selected entry");
    }
  }
  if (!entry.empty() && !found) throw std::runtime_error("missing entry");
  const std::set<std::string> allowed = {"iWrite", "iRead",    "iApply",
                                         "iReset", "iRunLoop", "iScan",
                                         "iClock", "iState"};
  Json::Elements operations;
  for (const auto& command : *commands) {
    if (command.words.empty()) continue;
    RequireStatic(command);
    const auto& name = command.words[0].value;
    if (name == "iNote" || name == "iPDLLevel" ||
        (entry.empty() && name == "iProcsForModule")) {
      continue;
    }
    if (!allowed.count(name)) {
      throw std::runtime_error("unsupported static trace command: " + name);
    }
    Json::Elements words;
    for (const auto& word : command.words) {
      words.push_back(Json::String(word.value));
    }
    operations.push_back(Json::Array(std::move(words)));
  }
  return Json::Object({{"binding", Json::String(binding)},
                       {"operations", Json::Array(std::move(operations))}});
}
}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 3) {
      throw std::runtime_error(
          "usage: pdl_pattern_trace FILE ENTRY [--apply-groups TOP --icl FILE "
          "...]");
    }
    auto trace = Trace(pdl::Document(ReadFile(argv[1])), argv[2]);
    if (argc > 3) {
      if (argc < 7 || std::string(argv[3]) != "--apply-groups")
        throw std::runtime_error("expected --apply-groups TOP --icl FILE");
      const std::string top = argv[4];
      pattern::Design design;
      for (int i = 5; i < argc;) {
        const std::string option = argv[i++];
        if (option == "--icl" && i < argc) {
          const std::string path = argv[i++];
          design.AddIcl(path, ReadFile(path));
        } else if (option == "--bsdl" && i + 1 < argc) {
          const std::string entity = argv[i++], path = argv[i++];
          design.AddBsdl(path, ReadFile(path), entity);
        } else {
          throw std::runtime_error("invalid comparison design option: " +
                                   option);
        }
      }
      trace = ApplyGroups(
          trace, pattern::ElaborateTapNetwork(design, top).netlist, top);
    }
    std::cout << trace.Serialize() << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
