// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <tcl.h>

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_clock.h"
#include "pattern/pattern_flatten.h"
#include "pattern/pattern_merge.h"
#include "pattern/pattern_opaque.h"
#include "pattern/pattern_readback.h"
#include "pattern/pattern_resources.h"
#include "pattern/pattern_runtime.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
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
  bool debug = false, physical = false, model_readback = false;
};
Options Parse(int argc, char** argv) {
  Options result;
  for (int i = 1; i < argc; ++i) {
    const std::string option = argv[i];
    if (option == "--model_readback") {
      result.model_readback = true;
      continue;
    }
    if (option == "--physical_pdl") {
      result.physical = true;
      continue;
    }
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
  if (result.physical && !result.local_reset.empty()) {
    throw std::runtime_error("--physical_pdl cannot use --local_reset");
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
  if (words.size() == 3 && words[0] == "iNote" &&
      (words[1] == "-comment" || words[1] == "-status")) {
    std::string result = "iNote " + words[1] + " \"";
    for (char c : words[2]) {
      if (c == '\n') {
        result += "\\n";
      } else if (c == '\r') {
        result += "\\r";
      } else if (c == '\t') {
        result += "\\t";
      } else {
        if (std::string("\\\"$[]").find(c) != std::string::npos) result += '\\';
        result += c;
      }
    }
    return result + "\"\n";
  }
  Tcl_DString list;
  Tcl_DStringInit(&list);
  for (const auto& word : words) Tcl_DStringAppendElement(&list, word.c_str());
  const std::string result = std::string(Tcl_DStringValue(&list)) + "\n";
  Tcl_DStringFree(&list);
  return result;
}
std::string OperationText(const pattern::Operation& operation,
                          bool modeled_named = false) {
  if (operation.command == "iGetStatus") {
    return Command({"iNote", "-comment",
                    pattern::StatusClearNoteText(operation.arguments.at(1))});
  }
  if (operation.command == "iNote" && operation.raw_history_omitted) return "";
  std::vector<std::string> words{operation.command};
  words.insert(words.end(), operation.arguments.begin(),
               operation.arguments.end());
  if (pattern::IsRawHistoryOmissionNote(operation) ||
      pattern::IsStatusClearNote(operation) ||
      (pattern::IsModeledChildHistoryNote(operation) &&
       !operation.modeled_raw_history)) {
    // Normalization or elision must not activate an unconsumed source note.
    words[2] = "informational " + words[2];
  }
  return (operation.command == "iScan" && operation.raw_history_omitted
              ? Command(
                    {"iNote", "-comment",
                     pattern::RawHistoryOmissionText(operation, modeled_named)})
              : "") +
         Command(words);
}
std::optional<std::string> GenerateOpaque(
    const pattern::Netlist& netlist,
    const std::vector<pattern::Operation>& operations) {
  const auto lowered = pattern::LowerOpaqueScans(netlist, operations);
  if (!lowered) return std::nullopt;
  std::string result =
      "iPDLLevel 0\n" + Command({"iProcsForModule", netlist.top_module});
  for (const auto& operation : *lowered) {
    result += OperationText(operation);
  }
  const pdl::Document validated(result);
  if (!validated.ok()) {
    throw std::runtime_error("generated opaque PDL failed shared parser: " +
                             validated.error_message());
  }
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
using ResetVariants = std::map<std::pair<size_t, size_t>,
                               std::map<std::vector<std::string>, std::string>>;
using ResetOccurrences = std::map<const pattern::SourceAction*, std::string>;
struct InvocationState {
  std::string setup, cleanup, verify;
};

InvocationState InvocationEdits(
    const std::vector<std::string>& sources,
    const std::vector<pattern::SourceAction>& actions,
    const ResetOccurrences& preambles, int final_level, ResetEdits* edits) {
  const std::string prefix = "IclPatternReset";
  std::set<size_t> used;
  for (const auto& source : sources) {
    size_t offset = 0;
    while ((offset = source.find(prefix, offset)) != std::string::npos) {
      offset += prefix.size();
      const auto end = source.find_first_not_of("0123456789", offset);
      const auto length =
          (end == std::string::npos ? source.size() : end) - offset;
      size_t value = 0;
      const auto parsed = std::from_chars(
          source.data() + offset, source.data() + offset + length, value);
      if (parsed.ec == std::errc{}) used.insert(value);
      offset += length;
    }
  }
  size_t suffix = 0;
  while (used.count(suffix)) ++suffix;
  const std::string name = "::" + prefix + std::to_string(suffix);
  const auto list = [](const std::vector<std::string>& words) {
    auto result = Command(words);
    result.pop_back();
    return result;
  };
  std::string setup = "if {[namespace exists " + name +
                      "]} {error {local-reset namespace collision}}\n" +
                      Command({"namespace", "eval", name, ""}) +
                      Command({"set", name + "::position(failed)", "0"});
  std::string cleanup =
      "\n\niPDLLevel 1\nif {[set " + name +
      "::position(failed)]} {error {local-reset invocation mismatch}}\n";
  std::map<std::pair<size_t, size_t>, std::vector<const pattern::SourceAction*>>
      sites;
  for (const auto& action : actions) {
    sites[{action.source, action.offset}].push_back(&action);
  }
  size_t id = 0;
  for (const auto& [site, calls] : sites) {
    const bool needed =
        std::any_of(calls.begin(), calls.end(), [&](const auto* call) {
          const auto found = preambles.find(call);
          return found != preambles.end() && !found->second.empty();
        });
    if (!needed) continue;
    if (site.first >= sources.size()) {
      throw std::runtime_error(
          "local-reset rewrite needs a literal source write/call site");
    }
    std::map<std::pair<std::vector<std::string>, std::string>, size_t> records;
    std::vector<std::string> values, sequence;
    for (const auto* call : calls) {
      const auto found = preambles.find(call);
      const std::string preamble =
          found == preambles.end() ? "" : found->second;
      const auto key = std::make_pair(call->arguments, preamble);
      const auto [entry, inserted] = records.emplace(key, values.size());
      if (inserted) values.push_back(list({list(call->arguments), preamble}));
      sequence.push_back(std::to_string(entry->second));
    }
    const auto index = "(" + std::to_string(id++) + ")";
    const auto position = name + "::position" + index;
    const auto record = name + "::records" + index;
    const auto order = name + "::sequence" + index;
    setup += Command({"set", position, "0"}) +
             Command({"set", record, list(values)}) +
             Command({"set", order, list(sequence)});
    (*edits)[site] = "set position [set " + position + "]\nset sequence [set " +
                     order +
                     "]\n"
                     "if {$position >= [llength $sequence]} {set " +
                     name +
                     "::position(failed) 1; error {unexpected "
                     "local-reset invocation}}\n"
                     "set record [lindex [set " +
                     record +
                     "] [lindex $sequence $position]]\n"
                     "if {[lrange $args 1 end] ne [lindex $record 0]} {set " +
                     name +
                     "::position(failed) 1; error "
                     "{unrecognized local-reset caller arguments}}\n"
                     "incr " +
                     position + "\neval [lindex $record 1]\n";
    cleanup += "if {[set " + position + "] != " + std::to_string(calls.size()) +
               "} {error {missing local-reset invocation}}\n";
    cleanup += "if {![string equal [set " + record + "] " +
               list({list(values)}) + "] || ![string equal [set " + order +
               "] " + list({list(sequence)}) +
               "]} {error {local-reset namespace interference}}\n";
  }
  if (!id) return {};
  const auto variables =
      list({name + "::position", name + "::records", name + "::sequence"});
  cleanup += "if {[namespace children " + name + "] ne {} || [info commands " +
             name + "::*] ne {} || [lsort [info vars " + name + "::*]] ne " +
             list({variables}) +
             "} {error {local-reset namespace interference}}\n";
  cleanup += "if {[array size " + name +
             "::position] != " + std::to_string(id + 1) + " || [array size " +
             name + "::records] != " + std::to_string(id) + " || [array size " +
             name + "::sequence] != " + std::to_string(id) +
             "} {error {local-reset namespace interference}}\n";
  cleanup += Command({"namespace", "delete", name});
  std::string verify =
      "iPDLLevel 1\nif {[namespace exists " + name +
      "]} {error {local-reset invocation cleanup did not complete}}\n";
  if (final_level != 1) {
    cleanup += Command({"iPDLLevel", std::to_string(final_level)});
    verify += Command({"iPDLLevel", std::to_string(final_level)});
  }
  return {setup, cleanup, verify};
}
std::string Generate(
    const pattern::Netlist& netlist,
    const std::vector<pattern::Operation>& operations,
    const std::vector<pattern::SourceAction>* actions = nullptr,
    ResetEdits* edits = nullptr,
    std::vector<pattern::Operation>* rewritten = nullptr,
    const pattern::ScanScheduler* initial = nullptr,
    const pattern::ResourceReservations* initial_reservations = nullptr,
    bool finalize = true, const pattern::SystemClocks* initial_clocks = nullptr,
    ResetVariants* variants = nullptr, bool physical = false,
    ResetOccurrences* occurrences = nullptr, bool modeled_readback = false) {
  pattern::ScanScheduler::Observation observe;
  if (modeled_readback) observe = [](const auto&, const auto&) {};
  if (physical) {
    const std::set<std::string> allowed = {"iWrite",   "iRead",     "iApply",
                                           "iReset",   "iNote",     "iTake",
                                           "iRelease", "iGetStatus"};
    for (const auto& op : operations) {
      if (op.callback) continue;
      if (!allowed.count(op.command)) {
        throw std::runtime_error(
            op.origin +
            ": unsupported operation in physical PDL: " + op.command);
      }
    }
  }
  if (!physical) {
    if (const auto opaque = GenerateOpaque(netlist, operations)) {
      if (rewritten) *rewritten = operations;
      return *opaque;
    }
  }
  pattern::ScanScheduler scheduler = initial ? pattern::ScanScheduler(*initial)
                                             : pattern::ScanScheduler(netlist);
  if (scheduler.HasOpaqueChild()) {
    for (const auto& operation : operations) {
      if (physical || operation.command == "iClock" ||
          operation.command == "iClockOverride" ||
          operation.command == "iOverrideScanInterface" ||
          operation.command == "iRunLoop") {
        throw std::runtime_error(operation.origin +
                                 ": unsupported opaque child execution mode");
      }
    }
  }
  pattern::ResourceReservations reservations =
      initial_reservations ? *initial_reservations
                           : pattern::ResourceReservations(netlist);
  pattern::SystemClocks clocks =
      initial_clocks ? *initial_clocks : pattern::SystemClocks(netlist);
  z3::context context;
  pattern::QueuedAccesses pending;
  std::map<std::tuple<bool, std::string, std::string>, pattern::Operation>
      pending_scans;
  size_t pending_scan_bits = 0;
  std::optional<pattern::Operation> last_root_scan;
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
      if (action.end > action.begin) {
        starts.emplace(action.begin, &action);
      } else if (variants) {
        (*variants)[{action.source, action.offset}].emplace(action.arguments,
                                                            "");
      }
    }
  }
  size_t covered = 0;
  for (size_t index = 0; index < operations.size(); ++index) {
    const auto& operation = operations[index];
    if (operation.command != "iNote") last_root_scan.reset();
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
          if (name == "iApply" || name == "iReset") queued = false;
          end = std::max(end, next + 1);
        }
        const std::vector<pattern::Operation> group(operations.begin() + index,
                                                    operations.begin() + end);
        bool ready = false;
        const auto contains = [&](const std::string& command) {
          return std::any_of(group.begin(), group.end(), [&](const auto& op) {
            return op.command == command;
          });
        };
        if (contains("iState") && (contains("iWrite") || contains("iRead"))) {
          try {
            ResetEdits strict;
            Generate(netlist, group, nullptr, &strict, nullptr, &scheduler,
                     &reservations, false, &clocks, nullptr, physical, nullptr,
                     modeled_readback);
            ready = true;
          } catch (const std::runtime_error&) {
            // A failed preview leaves the live state for prerequisite planning.
          }
        }
        const auto prerequisites =
            ready ? std::vector<std::vector<pattern::Operation>>{}
                  : scheduler.ResetPrerequisites(group, reservations.Values(),
                                                 reservations.UnwrittenBits());
        if (!prerequisites.empty() && !pending.Empty()) {
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
          scheduler.Apply(layer, false, reservations.Values(), false, physical,
                          observe);
          clocks.Invalidate();
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
          if (occurrences) {
            occurrences->emplace(&action, preamble);
            if (!preamble.empty()) {
              (*edits)[key] = preamble;
            } else {
              edits->emplace(key, "");
            }
          } else if (variants) {
            auto& alternatives = (*variants)[key];
            const auto previous = alternatives.find(action.arguments);
            if (previous != alternatives.end() &&
                previous->second != preamble) {
              throw std::runtime_error(
                  "local-reset identical caller arguments require different "
                  "releases on repeated execution");
            }
            if (!preamble.empty()) {
              (*edits)[key] = preamble;
            } else {
              edits->emplace(key, "");
            }
            alternatives.emplace(action.arguments, std::move(preamble));
          } else {
            const auto previous = edits->find(key);
            if (previous != edits->end() && previous->second != preamble) {
              throw std::runtime_error(
                  "local-reset source site requires different releases on "
                  "repeated execution");
            }
            edits->emplace(key, std::move(preamble));
          }
        }
        covered = end;
      }
      if (rewritten) rewritten->push_back(operation);
      if (operation.callback) continue;
      if (operation.command == "iWrite" || operation.command == "iRead") {
        // Reset may discard this queue, but malformed operands still fail.
        const auto target =
            pattern::ResolvePdlTarget(netlist, operation.arguments.at(0));
        pattern::ValidatePdlAccessTarget(netlist, target,
                                         operation.command == "iRead");
        const auto bits = operation.arguments.size() == 2
                              ? pattern::PdlDataValue(context, netlist, target,
                                                      operation.arguments.at(1))
                              : std::string(target.bits.size(), 'x');
        pending.Add(operation, target, bits);
      } else if (operation.command == "iScan") {
        auto scan = scheduler.NormalizeScan(operation, true);
        const auto fields = pattern::ReadScanFields(scan);
        const auto payload = [](const auto& queued) {
          return queued.arguments[pattern::ReadScanFields(queued).input]
                     .size() -
                 2;
        };
        const auto key = std::make_tuple(
            fields.instruction, scan.arguments[fields.target], fields.chain);
        const auto old = pending_scans.find(key);
        const size_t replaced =
            old == pending_scans.end() ? 0 : payload(old->second);
        const size_t size = pending_scan_bits - replaced + payload(scan);
        if (size > 1024 * 1024) {
          throw std::runtime_error("raw scan queued payload resource limit");
        }
        pending_scan_bits = size;
        pending_scans[key] = std::move(scan);
      } else if (operation.command == "iApply") {
        const auto caller_values =
            reservations.CallbackValues(operation, pending);
        const auto& preserved =
            caller_values ? *caller_values : reservations.Values();
        if (!operation.callback_preserved.empty() && !pending_scans.empty()) {
          throw std::runtime_error(
              "raw scans with callback caller goals are unsupported");
        }
        const bool together =
            pattern::ValidatePdlApplyOptions(operation.arguments);
        if (!pending_scans.empty()) {
          if (!pending.Empty()) {
            if (together) {
              throw std::runtime_error(
                  "mixed raw preparation cannot use iApply -together");
            }
            const auto& accesses = pending.Operations();
            if (edits) scheduler.CheckLocalResets(accesses);
            const auto steps = scheduler.Apply(accesses, false, preserved, true,
                                               physical, observe);
            clocks.Invalidate();
            reservations.Assigned(accesses, scheduler);
            append_steps(steps);
            pending.Clear();
          }
          std::vector<pattern::Operation> scans;
          scans.reserve(pending_scans.size());
          for (const auto& [name, scan] : pending_scans) scans.push_back(scan);
          std::set<size_t> assigned;
          std::vector<pattern::ScanStep> preparations;
          auto applied = scheduler.ApplyRawScans(
              scans, preserved, preserved.empty() ? nullptr : &assigned,
              &preparations,
              modeled_readback
                  ? pattern::ScanScheduler::RawObservation{[](const auto&,
                                                              const auto&) {}}
                  : pattern::ScanScheduler::RawObservation{});
          reservations.AssignedScans(assigned, scheduler);
          append_steps(preparations);
          for (const auto& scan : applied) {
            result += OperationText(
                scan, !pattern::ReadScanFields(scan).chain.empty());
          }
          result += together ? "iApply -together\n" : "iApply\n";
          if (applied.size() == 1) {
            for (const auto& scan : scans) {
              if (scan.arguments.size() < 6) continue;
              const auto client = netlist.node_ids.find(scan.arguments[0]);
              if (client == netlist.node_ids.end() ||
                  !netlist.nodes[client->second].scope) {
                continue;
              }
              pattern::Operation note{
                  "iNote",
                  {"-comment",
                   std::string(scheduler.DynamicRawHistory()
                                   ? pattern::kDynamicChildHistoryNote
                                   : pattern::kModeledChildHistoryNote) +
                       applied[0].arguments[0] + " " + scan.arguments[0] + " " +
                       scan.arguments[5]},
                  scan.origin,
                  scan.call_stack};
              if (scheduler.ModeledRawHistory(note, applied[0])) {
                note.modeled_raw_history = true;
                result += OperationText(note);
              }
            }
            last_root_scan = std::move(applied.front());
          }
          for (const auto& note : scheduler.OpaqueScanNotes()) {
            result += OperationText(note);
          }
          clocks.Invalidate();
          pending_scans.clear();
          pending_scan_bits = 0;
          continue;
        }
        const auto& accesses = pending.Operations();
        if (edits) scheduler.CheckLocalResets(accesses);
        const auto steps =
            scheduler.Apply(accesses, together, preserved, true, physical,
                            observe, operation.callback_preserved);
        clocks.Invalidate();
        reservations.Assigned(accesses, scheduler);
        append_steps(steps);
        if (steps.empty()) result += "iApply\n";
        pending.Clear();
      } else if (operation.command == "iReset") {
        const bool synchronous = !operation.arguments.empty();
        scheduler.Reset(synchronous, physical);
        clocks.Invalidate();
        reservations.Reset();
        pending.Clear();
        pending_scans.clear();
        pending_scan_bits = 0;
        result += synchronous ? "iReset -sync\n" : "iReset\n";
      } else if (operation.command == "iClockOverride") {
        clocks.Override(operation, context);
      } else if (operation.command == "iClock") {
        result += Command(
            {"iClock", clocks.Check(operation.arguments.at(0), scheduler)});
      } else if (operation.command == "iRunLoop") {
        const auto caller_values =
            reservations.CallbackValues(operation, pending);
        const auto& preserved =
            caller_values ? *caller_values : reservations.Values();
        if (!pending.Empty()) {
          throw std::runtime_error("iRunLoop has unapplied operations");
        }
        const auto& args = operation.arguments;
        if (args.size() == 2 && args[0] == "-time") {
          clocks.CheckTimeWait(preserved);
          clocks.Invalidate();
          result += Command({"iRunLoop", "-time", args[1]});
        } else {
          std::string literal, system_clock;
          if (args.size() == 3 && args[1] == "-sck") {
            literal = args[0];
            system_clock = args[2];
          } else if (args.size() == 1) {
            literal = args[0];
          } else if (args.size() == 2 && args[1] == "-tck") {
            literal = args[0];
          } else if (args.size() == 2 && args[0] == "-tck") {
            literal = args[1];
          } else {
            throw std::runtime_error(
                "iRunLoop -time requires absolute period models");
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
          if (system_clock.empty()) {
            scheduler.Idle(cycles, preserved);
            clocks.Invalidate();
            result += Command({"iRunLoop", std::to_string(cycles), "-tck"});
          } else {
            const auto wait =
                clocks.Wait(system_clock, cycles, scheduler, preserved);
            result += Command(
                {"iRunLoop", std::to_string(wait.second), "-sck", wait.first});
          }
        }
      } else if (operation.command == "iState") {
        const bool controlling = operation.arguments.size() != 3 ||
                                 operation.arguments[2] == "-LastWrittenValue";
        const auto& name = operation.arguments.at(0);
        const auto target = pattern::ResolvePdlTarget(netlist, name);
        const auto bits = pattern::PdlNumber(context, operation.arguments.at(1),
                                             target.bits.size());
        if (controlling) {
          scheduler.Assume(
              name, bits,
              reservations.CallbackConstraints(operation, pending, &scheduler));
          clocks.Invalidate();
          pending.Assume(target, bits);
          reservations.Assigned({operation}, scheduler);
          result += Command({"iState", name, "0b" + bits});
        } else {
          for (const auto& bit : target.bits) {
            pattern::ValidateStateTarget(netlist, bit.node);
          }
          result +=
              Command({"iState", name, "0b" + bits, operation.arguments[2]});
        }
      } else if (operation.command == "iTake" ||
                 operation.command == "iRelease") {
        reservations.Change(operation);
      } else if (operation.command == "iMerge" && edits) {
        // Reset-only rewriting retains the original operation indices/text.
        // Normal output uses MergeOperations to order notes and batch calls.
      } else if (operation.command == "iNote" ||
                 operation.command == "iGetStatus") {
        if (operation.command == "iNote") {
          scheduler.RestoreOpaqueScanNote(operation);
        }
        auto note = operation;
        if (pattern::IsModeledChildHistoryNote(note)) {
          note.modeled_raw_history =
              last_root_scan && note.modeled_history_source &&
              note.modeled_history_source ==
                  last_root_scan->modeled_history_source &&
              scheduler.ModeledRawHistory(note, *last_root_scan);
          if (!note.modeled_raw_history) last_root_scan.reset();
        } else {
          last_root_scan.reset();
        }
        result += OperationText(note);
      } else {
        throw std::runtime_error(
            "transaction scheduling is not implemented for " +
            operation.command);
      }
    } catch (const std::exception& error) {
      std::string message = operation.origin + ": " +
                            (operation.callback_preserved.empty()
                                 ? ""
                                 : "while preserving callback caller goals: ") +
                            error.what();
      for (const auto& frame : operation.call_stack) {
        message += "\n  while " + frame;
      }
      throw std::runtime_error(message);
    }
  }
  if (!pending.Empty() || !pending_scans.empty()) {
    throw std::runtime_error("unapplied PDL operations");
  }
  const bool had_transaction = std::any_of(
      operations.begin(), operations.end(), [](const auto& operation) {
        return operation.command == "iApply" || operation.command == "iReset" ||
               operation.command == "iRunLoop";
      });
  if (finalize && had_transaction) {
    try {
      append_steps(scheduler.Apply({}, false, reservations.Values(), false,
                                   physical, observe));
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
                           const std::vector<pattern::Operation>& operations,
                           bool modeled_readback = false, bool combine = true) {
  if (std::any_of(operations.begin(), operations.end(),
                  [](const auto& op) { return op.command == "iMerge"; }) &&
      pattern::ExpandOpaqueChild(netlist)) {
    combine = false;
  }
  const bool raw =
      std::any_of(operations.begin(), operations.end(),
                  [](const auto& op) { return op.command == "iScan"; });
  const bool modeled =
      std::any_of(netlist.nodes.begin(), netlist.nodes.end(),
                  [](const auto& node) { return node.kind == "ScanRegister"; });
  if (raw && modeled) {
    const std::set<std::string> allowed = {
        "iScan",  "iApply", "iReset",   "iRunLoop", "iNote",  "iRead",
        "iWrite", "iTake",  "iRelease", "iState",   "iMerge", "iGetStatus"};
    for (const auto& op : operations) {
      if (!allowed.count(op.command)) {
        throw std::runtime_error(
            op.origin +
            (pattern::ExpandOpaqueChild(netlist)
                 ? ": unsupported opaque child operation: "
                 : ": unsupported operation with modeled raw scans: ") +
            op.command);
      }
    }
  }
  const auto generate = [&](const auto& ordered) {
    return Generate(netlist, ordered, nullptr, nullptr, nullptr, nullptr,
                    nullptr, true, nullptr, nullptr, false, nullptr,
                    modeled_readback);
  };
  // Inspect raw operations before merging can erase unsupported interactions.
  if (const auto opaque = GenerateOpaque(netlist, operations)) return *opaque;
  if (std::none_of(operations.begin(), operations.end(),
                   [](const auto& op) { return op.command == "iMerge"; })) {
    return generate(operations);
  }
  try {
    return generate(pattern::MergeOperations(netlist, operations, combine));
  } catch (const std::runtime_error&) {
    if (!combine) throw;
    // A joint goal may be unreachable even when separate calls are valid.
    // Generate owns a fresh scheduler, so failed trials cannot leak state.
    return generate(pattern::MergeOperations(netlist, operations, false));
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
           "  --physical_pdl       Replay binary scan loads as one exact TAP "
           "CSU\n"
           "                       per iApply; use again when replaying "
           "output\n"
           "  --model_readback     Model register/PIO readback (no live "
           "device)\n"
           "                       Includes raw interface/chain history; "
           "opaque data is x.\n"
           "                       In physical mode, count observed comparison "
           "failures.\n"
           "  --debug              Also write debug/<top>.icl beside output\n"
           "  --log PATH           Copy diagnostics to a file\n"
           "  --local_reset PATH   Insert reset releases into the main "
           "sequence;\n"
           "                       preserve procedure definitions\n"
           "Current logical scheduling: standalone client chains, TAP scans,\n"
           "mixed IR/DR groups across distinct TAPs, mixed data access,\n"
           "logic/one-hot paths and local resets.\n"
           "Multiple TAPs require equivalent TCKs, disjoint lanes and explicit "
           "reset paths.\n"
           "Mixed IR/DR groups require distinct TMS pins.\n"
           "iApply -together requires a shared update cycle for queued "
           "writes.\n"
           "Nonzero TAP waits and TAP scans require explicit iReset.\n"
           "Standalone iRunLoop TCK waits need no reset. iClock and -sck "
           "support\n"
           "relative clock-only routes, differential partners, Period "
           "oscillators and versioned iClockOverride.\n"
           "Absolute-time waits require models without storage, clock-driven "
           "data,\n"
           "or forwarded test clocks.\n"
           "General clock events and stateful absolute time remain "
           "unsupported.\n"
           "Independent register and data/control-port iMerge calls can "
           "combine\n"
           "ordered applies, including reads, slices, aliases and masks.\n";
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
    if (options.model_readback) {
      messages += std::string(pattern::kModeledReadbackNote) + "\n";
    }
    if (!options.local_reset.empty()) ValidateResetSources(network.netlist);
    bool merge_readback_requested = false;
    size_t observed_cursor = 0;
    pattern::Runtime::Readback readback;
    pattern::Runtime::StatusReadback status_readback;
    pattern::Runtime::HistoryReadback history_readback;
    if (options.model_readback) {
      readback = [provider = pattern::MakeModeledReadback(
                      network.netlist, options.physical, &status_readback,
                      &history_readback),
                  &merge_readback_requested,
                  &observed_cursor](const auto& operations, bool clear_status) {
        while (!merge_readback_requested &&
               observed_cursor < operations.size()) {
          merge_readback_requested =
              operations[observed_cursor++].command == "iMerge";
        }
        return provider(operations, clear_status);
      };
    }
    pattern::Runtime runtime(design, network.netlist,
                             !options.local_reset.empty(), readback,
                             status_readback, history_readback);
    std::vector<std::string> sources;
    std::string local_reset;
    for (const auto& path : options.pdl) {
      const auto source = Read(path);
      runtime.Execute(path, source);
      if (!options.local_reset.empty()) sources.push_back(source);
    }
    runtime.Finish();
    if (runtime.CallbacksExecuted()) {
      if (options.model_readback &&
          std::any_of(runtime.operations().begin(), runtime.operations().end(),
                      [](const auto& operation) {
                        return operation.callback_prerequisite;
                      })) {
        if (!pattern::CanUsePrimaryCallbackReadback(network.netlist)) {
          throw std::runtime_error(
              "modeled readback excludes callback histories");
        }
        if (!options.physical) status_readback(runtime.operations());
      }
      if (options.physical) {
        throw std::runtime_error("physical PDL callbacks are unsupported");
      }
    }
    auto modeled_operations = std::vector<pattern::Operation>{};
    const auto* output_operations = &runtime.operations();
    const auto is_model_note = [](const auto& operation) {
      return operation.command == "iNote" && operation.arguments.size() == 1 &&
             (operation.arguments[0] == pattern::kModeledReadbackNote ||
              operation.arguments[0] ==
                  "modeled combinational PIO readback; no live device" ||
              operation.arguments[0] ==
                  "modeled PIO/DataRegister readback; no live device");
    };
    if (options.model_readback &&
        std::any_of(runtime.operations().begin(), runtime.operations().end(),
                    is_model_note)) {
      modeled_operations = runtime.operations();
      modeled_operations.erase(
          std::remove_if(modeled_operations.begin(), modeled_operations.end(),
                         is_model_note),
          modeled_operations.end());
      output_operations = &modeled_operations;
    }
    auto pdl =
        options.physical
            ? Generate(network.netlist, *output_operations, nullptr, nullptr,
                       nullptr, nullptr, nullptr, true, nullptr, nullptr, true,
                       nullptr, options.model_readback)
            : GenerateMerged(network.netlist, *output_operations,
                             options.model_readback, !merge_readback_requested);
    if (!options.local_reset.empty()) {
      const auto verify_source =
          [&](const std::string& source,
              const std::vector<pattern::Operation>& expected,
              const std::string& completion) {
            pattern::Runtime::StatusReadback verified_status;
            pattern::Runtime::HistoryReadback verified_history;
            const auto verified_readback =
                options.model_readback
                    ? pattern::MakeModeledReadback(
                          network.netlist, options.physical, &verified_status,
                          &verified_history)
                    : pattern::Runtime::Readback{};
            pattern::Runtime verified(design, network.netlist, false,
                                      verified_readback, verified_status,
                                      verified_history);
            verified.Execute(options.local_reset, source);
            if (verified.Level() != runtime.Level()) {
              throw std::runtime_error(
                  "local-reset insertion changes final PDL level");
            }
            if (!completion.empty()) {
              verified.Execute(options.local_reset + " completion", completion);
            }
            verified.Finish();
            const auto& actual = verified.operations();
            if (actual.size() != expected.size() ||
                !std::equal(actual.begin(), actual.end(), expected.begin(),
                            [](const auto& a, const auto& b) {
                              return a.command == b.command &&
                                     a.arguments == b.arguments;
                            })) {
              throw std::runtime_error(
                  "local-reset insertion changes source execution order or "
                  "targets");
            }
            ResetEdits strict;
            Generate(network.netlist, actual, nullptr, &strict, nullptr,
                     nullptr, nullptr, true, nullptr, nullptr, false, nullptr,
                     options.model_readback);
          };
      const auto rewrite = [&](bool whole_commands, bool after_arguments,
                               bool dispatch_arguments = false,
                               bool dispatch_occurrences = false) {
        std::string rewritten_source;
        ResetEdits edits;
        ResetVariants variants;
        ResetOccurrences occurrences;
        std::vector<pattern::Operation> expected;
        const auto actions = runtime.SourceActions(whole_commands);
        Generate(network.netlist, runtime.operations(), &actions, &edits,
                 &expected, nullptr, nullptr, true, nullptr,
                 dispatch_arguments ? &variants : nullptr, false,
                 dispatch_occurrences ? &occurrences : nullptr,
                 options.model_readback);
        const auto state = dispatch_occurrences
                               ? InvocationEdits(sources, actions, occurrences,
                                                 runtime.Level(), &edits)
                               : InvocationState{};
        rewritten_source = state.setup;
        for (const auto& [site, alternatives] : variants) {
          std::vector<std::string> branches;
          bool needed = false;
          for (const auto& [arguments, preamble] : alternatives) {
            auto key = Command(arguments);
            key.pop_back();
            branches.push_back(std::move(key));
            branches.push_back(preamble);
            needed = needed || !preamble.empty();
          }
          if (!needed) continue;
          branches.insert(
              branches.end(),
              {"default", "error {unrecognized local-reset caller arguments}"});
          edits[site] =
              "switch -exact -- [lrange $args 1 end] " + Command(branches);
        }
        if (after_arguments) {
          for (auto& [key, text] : edits) {
            if (text.empty()) continue;
            // Evaluate original words once before releases, in caller scope.
            text =
                Command({"apply", Command({"args", text + "uplevel 1 $args"})});
            text.back() = ' ';
          }
          for (const auto& action : actions) {
            const auto edit = edits.find({action.source, action.offset});
            if (edit == edits.end() || edit->second.empty()) continue;
            for (const auto position : action.vector_brackets) {
              edits.emplace(std::make_pair(action.source, position), "\\");
            }
          }
        }
        for (size_t source = 0; source < sources.size(); ++source) {
          size_t offset = 0;
          for (auto it = edits.lower_bound({source, 0});
               it != edits.end() && it->first.first == source; ++it) {
            const size_t position = it->first.second;
            if (position < offset || position > sources[source].size()) {
              throw std::runtime_error("invalid local-reset source range");
            }
            rewritten_source.append(sources[source], offset, position - offset);
            rewritten_source += it->second;
            offset = position;
          }
          rewritten_source.append(sources[source], offset, std::string::npos);
          // Separate file evaluation boundaries, including a continued EOF
          // comment.
          if (source + 1 < sources.size()) rewritten_source += "\n\n";
        }
        rewritten_source += state.cleanup;
        verify_source(rewritten_source, expected, state.verify);
        if (runtime.CallbacksExecuted()) {
          if (options.model_readback) {
            expected.erase(
                std::remove_if(expected.begin(), expected.end(), is_model_note),
                expected.end());
          }
          pdl =
              GenerateMerged(network.netlist, expected, options.model_readback,
                             !merge_readback_requested);
        }
        return rewritten_source;
      };
      try {
        for (size_t source = 0; source < sources.size(); ++source) {
          if (source) local_reset += "\n\n";
          local_reset += sources[source];
        }
        verify_source(local_reset, runtime.operations(), "");
      } catch (const std::runtime_error&) {
        try {
          local_reset = rewrite(true, false);
        } catch (const std::runtime_error&) {
          // Internal loop resets can require the original literal-site repair.
          try {
            local_reset = rewrite(false, false);
          } catch (const std::runtime_error&) {
            try {
              local_reset = rewrite(false, true);
            } catch (const std::runtime_error&) {
              try {
                local_reset = rewrite(false, true, true);
              } catch (const std::runtime_error&) {
                local_reset = rewrite(false, true, false, true);
              }
            }
          }
        }
      }
    }

    if (options.model_readback) {
      const auto header =
          "iPDLLevel 0\n" +
          Command({"iProcsForModule", network.netlist.top_module});
      if (pdl.compare(0, header.size(), header) != 0) {
        throw std::runtime_error("missing generated PDL header");
      }
      pdl.insert(header.size(),
                 Command({"iNote", pattern::kModeledReadbackNote}));
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
