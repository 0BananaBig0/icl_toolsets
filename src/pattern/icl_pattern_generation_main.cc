// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <tcl.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "pattern/pattern_flatten.h"
#include "pattern/pattern_merge.h"
#include "pattern/pattern_resources.h"
#include "pattern/pattern_runtime.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_tap.h"
#include "pattern/pattern_values.h"
#include "pdl/pdl_document.h"
#include "utility/file_output.h"
#include "utility/text_input.h"

namespace {
struct Options {
  std::vector<std::string> icl, pdl;
  std::vector<std::pair<std::string, std::string>> bsdl;
  std::string top, output, log, local_reset;
  bool debug = false;
};
Options Parse(int argc, char** argv) {
  Options result;
  for (int i = 1; i < argc; ++i) {
    const std::string option = argv[i];
    if (option == "--debug") {
      result.debug = true;
      continue;
    }
    if (option == "--bsdl") {
      if (i + 2 >= argc || std::string(argv[i + 1]).empty() ||
          std::string(argv[i + 2]).empty() ||
          std::string(argv[i + 1]).rfind("--", 0) == 0 ||
          std::string(argv[i + 2]).rfind("--", 0) == 0) {
        throw std::runtime_error("--bsdl requires ENTITY FILE");
      }
      result.bsdl.emplace_back(argv[i + 1], argv[i + 2]);
      i += 2;
      continue;
    }
    if (option == "--icl" || option == "--pdl") {
      auto& files = option == "--icl" ? result.icl : result.pdl;
      const size_t before = files.size();
      while (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) {
        files.push_back(argv[++i]);
      }
      if (files.size() == before) {
        throw std::runtime_error("missing files for " + option);
      }
      continue;
    }
    std::string* value = nullptr;
    if (option == "--top_module") {
      value = &result.top;
    } else if (option == "--output") {
      value = &result.output;
    } else if (option == "--log") {
      value = &result.log;
    } else if (option == "--local_reset") {
      value = &result.local_reset;
    } else {
      throw std::runtime_error("unknown option " + option);
    }
    if (!value->empty()) throw std::runtime_error("repeated option " + option);
    if (i + 1 == argc || std::string(argv[i + 1]).rfind("--", 0) == 0) {
      throw std::runtime_error("missing value for " + option);
    }
    *value = argv[++i];
    if (value->empty()) throw std::runtime_error("empty value for " + option);
  }
  if (result.icl.empty() || result.pdl.empty()) {
    throw std::runtime_error("--icl and --pdl inputs are required");
  }
  return result;
}
std::string Read(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  std::string text;
  if (!input || !toolsets::ReadTextStream(input, &text) ||
      text.find('\0') != std::string::npos) {
    throw std::runtime_error("cannot read text input " + path);
  }
  return text;
}
std::string Command(const std::vector<std::string>& words) {
  Tcl_DString list;
  Tcl_DStringInit(&list);
  for (const auto& word : words) Tcl_DStringAppendElement(&list, word.c_str());
  const std::string result = std::string(Tcl_DStringValue(&list)) + "\n";
  Tcl_DStringFree(&list);
  return result;
}
// Validate forwarding paths before solving instrument-driven reset cones.
void ValidateResetSources(const pattern::Netlist& netlist) {
  const size_t missing = std::numeric_limits<size_t>::max();
  std::set<size_t> verified;
  for (const auto& target : netlist.nodes) {
    if (!target.reset) continue;
    auto bit = *target.reset;
    std::set<size_t> path;
    while (bit.node != missing && !verified.count(bit.node)) {
      const auto& node = netlist.nodes.at(bit.node);
      if (!path.insert(bit.node).second) {
        throw std::runtime_error(node.origin + ": cyclic local-reset path at " +
                                 node.name);
      }
      if (node.kind != "ResetPort" && node.kind != "ToResetPort") {
        break;
      }
      const auto source = node.inputs.find("Source");
      if (source == node.inputs.end() && node.kind == "ResetPort" &&
          node.scope == 0) {
        break;
      }
      if (source == node.inputs.end() || source->second.size() != 1) {
        throw std::runtime_error(
            node.origin + ": unresolved local-reset path at " + node.name);
      }
      bit = source->second.front();
    }
    if (bit.node == missing &&
        bit.virtual_signal == pattern::VirtualSignal::kNone &&
        bit.constant != '0' && bit.constant != '1') {
      throw std::runtime_error(
          target.origin + ": unknown local-reset source for " + target.name);
    }
    verified.insert(path.begin(), path.end());
  }
}
using ResetEdits = std::map<std::pair<size_t, size_t>, std::string>;
std::string Generate(
    const pattern::Netlist& netlist,
    const std::vector<pattern::Operation>& operations,
    const std::vector<pattern::SourceAction>* actions = nullptr,
    ResetEdits* edits = nullptr,
    std::vector<pattern::Operation>* rewritten = nullptr) {
  pattern::ScanScheduler scheduler(netlist);
  pattern::ResourceReservations reservations(netlist);
  z3::context context;
  std::vector<pattern::Operation> pending;
  std::string result =
      "iPDLLevel 0\n" + Command({"iProcsForModule", netlist.top_module});
  const auto append_steps = [&](const std::vector<pattern::ScanStep>& steps) {
    for (const auto& step : steps) {
      if (step.writes.empty() && step.expected.empty()) {
        throw std::runtime_error(
            "scan transaction has no expressible PDL update");
      }
      for (const auto& [name, bits] : step.writes) {
        result += Command({"iWrite", name, "0b" + bits});
      }
      for (const auto& [name, bits] : step.expected) {
        result += Command({"iRead", name, "0b" + bits});
      }
      result += step.together ? "iApply -together\n" : "iApply\n";
    }
  };
  std::map<size_t, const pattern::SourceAction*> starts;
  if (actions) {
    for (const auto& action : *actions) {
      if (action.end > action.begin) starts.emplace(action.begin, &action);
    }
  }
  size_t covered = 0;
  for (size_t index = 0; index < operations.size(); ++index) {
    const auto& operation = operations[index];
    try {
      reservations.Enter(operation.call_stack);
      const auto entry = starts.find(index);
      if (edits && entry != starts.end() && index < covered) {
        const auto& action = *entry->second;
        const auto key = std::make_pair(action.source, action.offset);
        const auto previous = edits->find(key);
        if (previous != edits->end() && !previous->second.empty()) {
          throw std::runtime_error(
              "local-reset insertion would split a repeatedly executed pending "
              "group");
        }
        edits->emplace(key, "");
      }
      if (edits && entry != starts.end() && index >= covered) {
        const auto& action = *entry->second;
        size_t end = action.end;
        bool queued = false;
        // Hoist prerequisites before the whole queue, never insert an apply
        // that would flush a preceding user write early.
        for (size_t next = index; next < end || queued; ++next) {
          if (next >= operations.size()) {
            throw std::runtime_error("unapplied reset rewrite group");
          }
          const auto nested = starts.find(next);
          if (nested != starts.end()) end = std::max(end, nested->second->end);
          const auto& name = operations[next].command;
          if (name == "iWrite" || name == "iRead") queued = true;
          if (name == "iApply") queued = false;
          end = std::max(end, next + 1);
        }
        const std::vector<pattern::Operation> group(operations.begin() + index,
                                                    operations.begin() + end);
        const auto prerequisites = scheduler.ResetPrerequisites(
            group, reservations.Values(), reservations.UnwrittenRegisters());
        if (!prerequisites.empty() && !pending.empty()) {
          throw std::runtime_error(
              "local-reset insertion would flush pending user operations");
        }
        std::string preamble;
        if (!prerequisites.empty() && !action.prefix.empty()) {
          preamble += "iPrefix -\n";
        }
        for (const auto& layer : prerequisites) {
          reservations.CheckAutomaticWrites(layer);
          scheduler.CheckLocalResets(layer);
          scheduler.Apply(layer, false, reservations.Values(), false);
          if (rewritten) {
            rewritten->insert(rewritten->end(), layer.begin(), layer.end());
            rewritten->push_back({"iApply", {}, operation.origin, {}});
          }
          for (const auto& write : layer) {
            preamble +=
                Command({"iWrite", write.arguments[0], write.arguments[1]});
          }
          preamble += "iApply\n";
        }
        if (!prerequisites.empty() && !action.prefix.empty()) {
          preamble += Command({"iPrefix", action.prefix});
        }
        if (action.source == std::numeric_limits<size_t>::max()) {
          if (!preamble.empty()) {
            throw std::runtime_error(
                "local-reset rewrite needs a literal source write/call site");
          }
        } else {
          const auto key = std::make_pair(action.source, action.offset);
          const auto previous = edits->find(key);
          if (previous != edits->end() && previous->second != preamble) {
            throw std::runtime_error(
                "local-reset source site requires different releases on "
                "repeated execution");
          }
          edits->emplace(key, std::move(preamble));
        }
        covered = end;
      }
      if (rewritten) rewritten->push_back(operation);
      if (operation.command == "iWrite" || operation.command == "iRead") {
        pending.push_back(operation);
      } else if (operation.command == "iApply") {
        const bool together =
            operation.arguments == std::vector<std::string>{"-together"};
        if (!operation.arguments.empty() && !together) {
          throw std::runtime_error("unsupported iApply option");
        }
        if (edits) scheduler.CheckLocalResets(pending);
        const auto steps =
            scheduler.Apply(pending, together, reservations.Values());
        reservations.Assigned(pending, scheduler);
        append_steps(steps);
        if (steps.empty()) result += "iApply\n";
        pending.clear();
      } else if (operation.command == "iReset") {
        if (!reservations.Values().empty()) {
          throw std::runtime_error(
              "iReset with written reservations is not "
              "implemented; release resources first");
        }
        if (!pending.empty()) {
          throw std::runtime_error(
              "iReset with pending actions requires queued-state integration");
        }
        const bool synchronous = !operation.arguments.empty();
        scheduler.Reset(synchronous);
        result += synchronous ? "iReset -sync\n" : "iReset\n";
      } else if (operation.command == "iRunLoop") {
        if (!pending.empty()) {
          throw std::runtime_error("iRunLoop has unapplied operations");
        }
        const auto& args = operation.arguments;
        std::string literal;
        if (args.size() == 1) {
          literal = args[0];
        } else if (args.size() == 2 && args[1] == "-tck") {
          literal = args[0];
        } else if (args.size() == 2 && args[0] == "-tck") {
          literal = args[1];
        } else {
          throw std::runtime_error(
              "iRunLoop -sck/-time requires system-clock and period models");
        }
        if (literal.find_first_of("xX", 2) != std::string::npos) {
          throw std::runtime_error("iRunLoop cycle count must be known");
        }
        const auto bits = pattern::PdlNumber(context, literal, 64);
        uint64_t cycles = 0;
        for (char bit : bits) {
          if (bit == 'x') {
            throw std::runtime_error("iRunLoop cycle count must be known");
          }
          cycles = (cycles << 1) | (bit == '1');
        }
        scheduler.Idle(cycles, reservations.Values());
        result += Command({"iRunLoop", std::to_string(cycles), "-tck"});
      } else if (operation.command == "iState") {
        if (!pending.empty()) {
          throw std::runtime_error(
              "iState with pending actions requires queued-state integration");
        }
        if (operation.arguments.size() == 3 &&
            operation.arguments[2] != "-LastWrittenValue") {
          throw std::runtime_error(
              "readback iState options require observation-state integration");
        }
        const auto& name = operation.arguments.at(0);
        const auto target = pattern::ResolvePdlTarget(netlist, name);
        const auto bits = pattern::PdlNumber(context, operation.arguments.at(1),
                                             target.bits.size());
        scheduler.Assume(name, bits);
        reservations.Assigned({operation}, scheduler);
        result += Command({"iState", name, "0b" + bits});
      } else if (operation.command == "iTake" ||
                 operation.command == "iRelease") {
        reservations.Change(operation);
      } else if (operation.command == "iMerge" && edits) {
        // Reset-only rewriting retains the original operation indices/text.
        // Normal output uses MergeOperations to order notes and batch calls.
      } else if (operation.command == "iNote") {
        std::vector<std::string> words{operation.command};
        words.insert(words.end(), operation.arguments.begin(),
                     operation.arguments.end());
        result += Command(words);
      } else {
        throw std::runtime_error(
            "transaction scheduling is not implemented for " +
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
  if (!pending.empty()) throw std::runtime_error("unapplied PDL operations");
  if (!operations.empty()) {
    try {
      append_steps(scheduler.Apply({}, false, reservations.Values(), false));
    } catch (const std::exception& error) {
      const auto& last = operations.back();
      std::string message =
          last.origin + ": final iApplyEndState restoration: " + error.what();
      for (const auto& frame : last.call_stack) message += "\n  while " + frame;
      throw std::runtime_error(message);
    }
  }
  const pdl::Document validated(result);
  if (!validated.ok()) {
    throw std::runtime_error("generated PDL failed shared parser: " +
                             validated.error_message());
  }
  return result;
}
std::string GenerateMerged(const pattern::Netlist& netlist,
                           const std::vector<pattern::Operation>& operations) {
  if (std::none_of(operations.begin(), operations.end(),
                   [](const auto& op) { return op.command == "iMerge"; })) {
    return Generate(netlist, operations);
  }
  try {
    return Generate(netlist,
                    pattern::MergeOperations(netlist, operations, true));
  } catch (const std::runtime_error&) {
    // A joint goal may be unreachable even when separate calls are valid.
    // Generate owns a fresh scheduler, so failed trials cannot leak state.
    return Generate(netlist,
                    pattern::MergeOperations(netlist, operations, false));
  }
}
void CheckPaths(const std::vector<std::string>& inputs,
                const std::vector<std::string>& outputs) {
  std::vector<std::filesystem::path> previous;
  for (const auto& input : inputs) previous.push_back(input);
  for (const auto& output : outputs) {
    if (output.empty()) continue;
    const auto canonical = std::filesystem::weakly_canonical(output);
    for (const auto& path : previous) {
      std::error_code error;
      if (canonical == std::filesystem::weakly_canonical(path) ||
          std::filesystem::equivalent(output, path, error)) {
        throw std::runtime_error("output aliases another input/output: " +
                                 output);
      }
    }
    previous.push_back(output);
  }
}
void Write(const std::string& path, const std::string& text) {
  const auto parent = std::filesystem::path(path).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  std::string error;
  if (!toolsets::WriteTextFile(path, text, &error)) {
    throw std::runtime_error(error);
  }
}
}  // namespace
int main(int argc, char** argv) {
  if (argc == 2 &&
      (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
    std::cout
        << "icl-pattern-generation " << ICL_FORMAT_VERSION
        << "\n"
           "Usage: icl-pattern-generation --icl FILE... --pdl FILE... "
           "[options]\n"
           "  --bsdl ENTITY FILE   Load an AccessLink BSDL model (repeatable)\n"
           "  --top_module NAME    Select the elaborated top (default: unique "
           "root)\n"
           "  --output PATH        Output PDL (default: out/<top>.pdl)\n"
           "  --debug              Also write debug/<top>.icl beside output\n"
           "  --log PATH           Copy diagnostics to a file\n"
           "  --local_reset PATH   Insert reset releases into the main "
           "sequence;\n"
           "                       preserve procedure definitions\n"
           "Current scheduling: ordinary single SI/SO, scan registers, muxes,\n"
           "logic/one-hot paths and local resets; literal "
           "iWrite/iApply/iState.\n"
           "iApply -together requires a shared update cycle for queued "
           "writes.\n"
           "Single-TAP IR/DR scans and nonzero iRunLoop TCK counts require\n"
           "explicit iReset. iRunLoop -sck/-time, multi-TAP, shadow\n"
           "register transactions and free-running clocks remain unsupported.\n"
           "Independent full-register write-only iMerge calls can combine "
           "ordered apply groups.\n";
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--version") {
    std::cout << "icl-pattern-generation " << ICL_FORMAT_VERSION << "\n";
    return 0;
  }
  std::string log_path, messages;
  try {
    const auto options = Parse(argc, argv);
    std::vector<std::string> inputs = options.icl;
    inputs.insert(inputs.end(), options.pdl.begin(), options.pdl.end());
    for (const auto& [entity, path] : options.bsdl) inputs.push_back(path);
    CheckPaths(inputs, {options.output, options.log, options.local_reset});
    // Log output becomes writable only after alias validation.
    log_path = options.log;
    pattern::Design design;
    for (const auto& path : options.icl) design.AddIcl(path, Read(path));
    for (const auto& [entity, path] : options.bsdl) {
      design.AddBsdl(path, Read(path), entity);
    }
    const std::string top = design.SelectTop(options.top).name();
    const auto output =
        options.output.empty() ? "out/" + top + ".pdl" : options.output;
    const auto debug = options.debug
                           ? (std::filesystem::path(output).parent_path() /
                              "debug" / (top + ".icl"))
                                 .string()
                           : "";
    log_path.clear();
    CheckPaths(inputs, {output, debug, options.log, options.local_reset});
    log_path = options.log;
    const auto network = pattern::ElaborateTapNetwork(design, top);
    if (!options.local_reset.empty()) ValidateResetSources(network.netlist);
    pattern::Runtime runtime(design, network.netlist,
                             !options.local_reset.empty());
    std::vector<std::string> sources;
    std::string local_reset;
    for (const auto& path : options.pdl) {
      const auto source = Read(path);
      runtime.Execute(path, source);
      if (!options.local_reset.empty()) sources.push_back(source);
    }
    runtime.Finish();
    const auto pdl = GenerateMerged(network.netlist, runtime.operations());
    if (!options.local_reset.empty()) {
      ResetEdits edits;
      std::vector<pattern::Operation> expected;
      Generate(network.netlist, runtime.operations(), &runtime.SourceActions(),
               &edits, &expected);
      for (size_t source = 0; source < sources.size(); ++source) {
        size_t offset = 0;
        for (auto it = edits.lower_bound({source, 0});
             it != edits.end() && it->first.first == source; ++it) {
          const size_t position = it->first.second;
          if (position < offset || position > sources[source].size()) {
            throw std::runtime_error("invalid local-reset source range");
          }
          local_reset.append(sources[source], offset, position - offset);
          local_reset += it->second;
          offset = position;
        }
        local_reset.append(sources[source], offset, std::string::npos);
        // Separate file evaluation boundaries, including a continued EOF
        // comment.
        if (source + 1 < sources.size()) local_reset += "\n\n";
      }
      pattern::Runtime verified(design, network.netlist);
      verified.Execute(options.local_reset, local_reset);
      verified.Finish();
      const auto& actual = verified.operations();
      if (actual.size() != expected.size() ||
          !std::equal(actual.begin(), actual.end(), expected.begin(),
                      [](const auto& a, const auto& b) {
                        return a.command == b.command &&
                               a.arguments == b.arguments;
                      })) {
        throw std::runtime_error(
            "local-reset insertion changes source execution order or targets");
      }
    }
    for (const auto& warning : design.warnings()) messages += warning + "\n";
    for (const auto& warning : runtime.warnings()) messages += warning + "\n";
    // All parsing, execution, scheduling and replay finish before publication.
    const auto success_messages = messages + "info: generated " + output + "\n";
    std::vector<std::pair<std::string, std::string>> artifacts;
    if (!debug.empty()) {
      artifacts.emplace_back(debug, pattern::SerializeIcl(network.netlist));
    }
    if (!options.local_reset.empty()) {
      artifacts.emplace_back(options.local_reset, local_reset);
    }
    artifacts.emplace_back(output, pdl);
    if (!log_path.empty()) artifacts.emplace_back(log_path, success_messages);
    std::vector<std::unique_ptr<toolsets::StagedTextFile>> staged;
    for (const auto& [path, data] : artifacts) {
      const auto parent = std::filesystem::path(path).parent_path();
      if (!parent.empty()) std::filesystem::create_directories(parent);
      auto file = std::make_unique<toolsets::StagedTextFile>();
      std::string error;
      if (!file->Prepare(path, data, &error)) throw std::runtime_error(error);
      staged.push_back(std::move(file));
    }
    for (const auto& file : staged) {
      std::string error;
      if (!file->Publish(&error)) {
        throw std::runtime_error(
            error + "; earlier artifact renames may have completed");
      }
    }
    std::cout << success_messages;
    return std::cout ? 0 : 2;
  } catch (const std::exception& error) {
    messages += std::string("error: ") + error.what() + "\n";
    if (!log_path.empty()) {
      try {
        Write(log_path, messages);
      } catch (const std::exception& log_error) {
        messages += std::string("error: ") + log_error.what() + "\n";
      }
    }
    std::cout << messages;
    return 2;
  }
}
