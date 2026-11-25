// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_runtime.h"

#include <tcl.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_netlist.h"
#include "pattern/pattern_opaque.h"
#include "pattern/pattern_readback.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"
#include "pdl/pdl_document.h"

namespace pattern {
namespace {
std::string JoinPath(const std::string& prefix, const std::string& name) {
  if (prefix.empty()) return name;
  return name.empty() ? prefix : prefix + "." + name;
}

std::string CommandName(std::string name) {
  if (name.rfind("::", 0) == 0) name.erase(0, 2);
  return name;
}
}  // namespace

class Runtime::Impl {
 public:
  Impl(const Design& design, const std::string& top,
       const Netlist* netlist = nullptr, bool trace_source = false,
       Readback readback = {}, StatusReadback status_readback = {},
       HistoryReadback history_readback = {})
      : trace_source_(trace_source),
        readback_(std::move(readback)),
        status_readback_(std::move(status_readback)),
        history_readback_(std::move(history_readback)),
        design_(design),
        netlist_(netlist),
        top_(top),
        master_(Tcl_CreateInterp(), Tcl_DeleteInterp),
        interpreter_(Tcl_CreateSlave(master_.get(), "pattern", 1),
                     Tcl_DeleteInterp) {
    if (!design_.modules().count(top_)) {
      throw std::runtime_error("unknown runtime top module " + top_);
    }
    if (!interpreter_) {
      throw std::runtime_error("cannot create safe PDL interpreter");
    }
    // Tcl deadlines cannot interrupt native waits or instrument child
    // interpreters.
    for (const char* command : {"after", "vwait", "update", "interp"}) {
      Tcl_DeleteCommand(interpreter_.get(), command);
    }
    Tcl_SetRecursionLimit(interpreter_.get(), kRecursionLimit);
    Tcl_CreateObjTrace(interpreter_.get(), 0, 0, Trace, this, nullptr);
    for (const auto& name : pdl::Keywords()) {
      if (!pdl::FindCommandSpec(name)) continue;
      Tcl_CreateObjCommand(interpreter_.get(), name.c_str(), Dispatch, this,
                           nullptr);
    }
    Tcl_CreateObjCommand(interpreter_.get(), "::icl_pattern_dispatch", Dispatch,
                         this, nullptr);
    Tcl_CreateObjCommand(interpreter_.get(), "::icl_pattern_scope", Scope, this,
                         nullptr);
    frames_.push_back({});
    if (netlist) {
      has_callbacks_ = std::any_of(
          netlist->nodes.begin(), netlist->nodes.end(), [](const auto& node) {
            return node.read_callback || node.write_callback;
          });
      for (const auto& node : netlist->nodes) {
        if (!node.read_callback && !node.write_callback) continue;
        size_t scope = node.scope;
        while (true) {
          callback_scopes_.insert(netlist->scopes.at(scope).path);
          if (scope == 0) break;
          scope = netlist->scopes.at(scope).parent;
        }
      }
      module_parameters_ = netlist->module_parameters;
      for (const auto& scope : netlist->scopes) {
        instance_scopes_.emplace(scope.path, &scope);
      }
    }
  }

  void Execute(const std::string& path, const std::string& text) {
    if (failed_) {
      throw std::runtime_error("PDL runtime cannot resume after an error");
    }
    if (finished_) throw std::runtime_error("PDL runtime is finished");
    const auto document = std::make_shared<pdl::Document>(
        text, false,
        pdl::ParseContext{level_, binding_, definition_namespace_, false});
    if (!document->ok()) {
      failed_ = true;
      throw std::runtime_error(path + ":" +
                               std::to_string(document->error_line()) + ": " +
                               document->error_message());
    }
    fallback_origin_ = path + ":1:1";
    const auto script = Instrument(path, document, source_count_++);
    Tcl_Time deadline;
    Tcl_GetTime(&deadline);
    deadline.sec += 5;
    Tcl_LimitSetTime(interpreter_.get(), &deadline);
    Tcl_LimitTypeSet(interpreter_.get(), TCL_LIMIT_TIME);
    const int result =
        Tcl_EvalEx(interpreter_.get(), script.data(),
                   static_cast<int>(script.size()), TCL_EVAL_GLOBAL);
    CloseScope();
    if (result != TCL_OK) {
      failed_ = true;
      throw std::runtime_error(path + ": " +
                               Tcl_GetStringResult(interpreter_.get()));
    }
  }

  void Finish() {
    if (failed_) {
      throw std::runtime_error("PDL runtime cannot finish after an error");
    }
    if (!merges_.empty()) {
      failed_ = true;
      throw std::runtime_error(merges_.back().origin + ": unclosed iMerge");
    }
    if (callback_failed_) {
      failed_ = true;
      throw std::runtime_error("failed callback queue requires iReset");
    }
    if (pending_) {
      failed_ = true;
      const auto& operation = operations_.at(pending_operation_);
      std::string message =
          operation.origin + ": unapplied " + operation.command;
      for (const auto& frame : operation.call_stack) {
        message += "\n  while " + frame;
      }
      throw std::runtime_error(message);
    }
    finished_ = true;
  }

  const std::vector<Operation>& operations() const { return operations_; }
  bool CallbacksExecuted() const { return callbacks_executed_; }
  bool ReadCallbacksExecuted() const { return read_callbacks_executed_; }
  const std::vector<std::string>& warnings() const { return warnings_; }
  int Level() const { return level_; }
  std::vector<SourceAction> SourceActions(bool whole_commands) const {
    auto actions = actions_;
    for (const auto& [marker, end] : callback_action_ends_) {
      auto owner =
          std::upper_bound(actions.begin(), actions.end(), marker,
                           [](size_t index, const SourceAction& action) {
                             return index < action.begin;
                           });
      if (owner == actions.begin()) continue;
      --owner;
      if (owner->end > marker) owner->end = std::max(owner->end, end);
    }
    if (!whole_commands) return actions;
    std::map<size_t, SourceAction> starts;
    for (const auto& action : actions) {
      if (action.end > action.begin) starts.emplace(action.begin, action);
    }
    for (auto action : compounds_) {
      if (action.end == action.begin) continue;
      auto next = starts.lower_bound(action.begin);
      if (next != starts.begin()) {
        const auto previous = std::prev(next);
        if (previous->second.end > action.begin) {
          const size_t end = std::max(action.end, previous->second.end);
          action = previous->second;
          action.end = end;
          starts.erase(previous);
        }
      }
      while (next != starts.end() && next->first < action.end) {
        // A merge opened inside this command may end in a later source file.
        action.end = std::max(action.end, next->second.end);
        next = starts.erase(next);
      }
      starts.emplace(action.begin, std::move(action));
    }
    std::vector<SourceAction> result;
    for (const auto& [begin, action] : starts) result.push_back(action);
    return result;
  }

 private:
  struct Site {
    std::string path;
    std::shared_ptr<const pdl::Document> document;
    pdl::ByteRange range;
    const pdl::Command* command;
    size_t source;
    bool immediate_merge = false;
    std::string Location() const {
      const auto& lines = document->line_starts();
      const auto line =
          std::upper_bound(lines.begin(), lines.end(), range.begin);
      return path + ":" + std::to_string(line - lines.begin()) + ":" +
             std::to_string(range.begin - *(line - 1) + 1);
    }
  };
  struct Frame {
    std::string instance, prefix, procedure_namespace, invocation;
    size_t command = 0;
    int level = 1;
  };
  static constexpr size_t kRecursionLimit = 128;
  struct Procedure {
    std::string tcl_name, origin;
    int level;
    std::vector<std::string> parameters;
    size_t minimum_arguments = 0, maximum_arguments = 0;
  };
  using ProcedureKey = std::tuple<std::string, std::string, std::string>;
  struct MergeCall {
    Procedure definition;
    std::string instance;
    std::vector<std::string> arguments;
    std::string invocation, origin;
  };

  std::string Instrument(const std::string& path,
                         const std::shared_ptr<const pdl::Document>& document,
                         size_t source = std::numeric_limits<size_t>::max()) {
    struct Edit {
      size_t end;
      std::string text;
    };
    std::map<size_t, Edit> edits;
    std::function<void(const std::vector<pdl::Command>&, bool, bool)> visit;
    visit = [&](const auto& commands, bool procedure, bool top) {
      std::optional<size_t> plain_merge;
      for (const auto& command : commands) {
        if (command.words.empty()) continue;
        const auto& first = command.words.front();
        const auto name = CommandName(first.value);
        const bool is_pdl =
            first.literal && !first.expanded && pdl::FindCommandSpec(name);
        if (plain_merge &&
            (!is_pdl ||
             (name != "iCall" && name != "iNote" && name != "iMerge") ||
             std::any_of(command.words.begin(), command.words.end(),
                         [&](const auto& word) {
                           return !word.substitutions.empty() ||
                                  (name == "iNote" &&
                                   (!word.literal || word.expanded));
                         }))) {
          plain_merge.reset();
        }
        if (plain_merge && name == "iNote") {
          std::vector<std::string> arguments;
          for (size_t i = 1; i < command.words.size(); ++i) {
            arguments.push_back(command.words[i].value);
          }
          const Operation note{"iNote", arguments, "", {}};
          if (CallbackHistoryNote(arguments) || RawOmissionNote(note) ||
              OpaqueStableNote(note) ||
              (netlist_ && ReadOpaqueChildHistoryNote(*netlist_, note))) {
            // Model notes can restore state or bind to the following scan.
            plain_merge.reset();
          }
        }
        if (is_pdl) {
          const size_t id = sites_.size();
          sites_.push_back(
              {path, document, command.range, &command,
               procedure ? std::numeric_limits<size_t>::max() : source});
          if (name == "iMerge" && command.words.size() == 2 &&
              command.words[1].literal) {
            if (command.words[1].value == "-begin") {
              plain_merge = id;
            } else if (command.words[1].value == "-end" && plain_merge) {
              sites_[*plain_merge].immediate_merge = true;
              plain_merge.reset();
            }
          }
          edits.insert_or_assign(
              first.range.begin,
              Edit{first.range.end, "::icl_pattern_dispatch " +
                                        std::to_string(id) + " " + name});
          for (const auto& word : command.words) {
            for (const auto& range : word.vector_ranges) {
              edits.insert_or_assign(range.begin, Edit{range.begin, "\\"});
              edits.insert_or_assign(range.end - 1, Edit{range.end - 1, "\\"});
            }
          }
        } else if (top && trace_source_ &&
                   source != std::numeric_limits<size_t>::max()) {
          const size_t id = sites_.size();
          sites_.push_back({path, document, command.range, &command, source});
          edits.emplace(
              first.range.begin,
              Edit{first.range.begin,
                   "::icl_pattern_scope " + std::to_string(id) + ";\n"});
          const size_t end = command.words.back().range.end;
          edits.emplace(end, Edit{end, "\n::icl_pattern_scope -1;"});
        }
        for (const auto& word : command.words) {
          visit(word.script, procedure || name == "proc" || name == "iProc",
                false);
          visit(word.substitutions, procedure, false);
        }
      }
    };
    visit(document->commands(), false, true);
    std::string result;
    size_t begin = 0;
    for (const auto& entry : edits) {
      if (entry.first < begin) continue;
      result.append(document->source(), begin, entry.first - begin);
      result += entry.second.text;
      begin = entry.second.end;
    }
    result.append(document->source(), begin, std::string::npos);
    return result;
  }

  void CloseScope() {
    if (!compound_) return;
    compound_->end = operations_.size();
    compounds_.push_back(std::move(*compound_));
    compound_.reset();
  }

  static int Scope(void* data, Tcl_Interp* interpreter, int count,
                   Tcl_Obj* const values[]) {
    auto& self = *static_cast<Impl*>(data);
    Tcl_WideInt id = -2;
    if (count != 2 ||
        Tcl_GetWideIntFromObj(interpreter, values[1], &id) != TCL_OK ||
        id < -1 || (id >= 0 && static_cast<size_t>(id) >= self.sites_.size())) {
      Tcl_SetObjResult(interpreter,
                       Tcl_NewStringObj("invalid PDL compound identity", -1));
      return TCL_ERROR;
    }
    try {
      self.CloseScope();
      if (id >= 0 && self.trace_source_ && self.frames_.size() == 1) {
        const auto& site = self.sites_[static_cast<size_t>(id)];
        self.compound_ = SourceAction{
            site.source, site.range.begin,           self.operations_.size(),
            0,           self.frames_.back().prefix, {}};
      }
      return TCL_OK;
    } catch (const std::exception& error) {
      Tcl_SetObjResult(interpreter, Tcl_NewStringObj(error.what(), -1));
      return TCL_ERROR;
    }
  }

  static int Dispatch(void* data, Tcl_Interp* interpreter, int count,
                      Tcl_Obj* const values[]) {
    auto& self = *static_cast<Impl*>(data);
    std::string origin = self.fallback_origin_;
    try {
      std::optional<Site> site;
      int start = 0;
      if (std::string(Tcl_GetString(values[0])) == "::icl_pattern_dispatch") {
        Tcl_WideInt id = -1;
        if (count < 3 ||
            Tcl_GetWideIntFromObj(interpreter, values[1], &id) != TCL_OK ||
            id < 0 || static_cast<size_t>(id) >= self.sites_.size()) {
          throw std::runtime_error("invalid PDL source identity");
        }
        site = self.sites_[static_cast<size_t>(id)];
        origin = site->Location();
        start = 2;
      }
      std::vector<std::string> arguments;
      for (int i = start; i < count; ++i) {
        decltype(Tcl_Parse{}.numWords) length = 0;
        const auto* characters = Tcl_GetUnicodeFromObj(values[i], &length);
        if (std::find(characters, characters + length, 0) !=
            characters + length) {
          throw std::runtime_error("NUL is not allowed in PDL arguments");
        }
        const auto* bytes = Tcl_GetStringFromObj(values[i], &length);
        arguments.emplace_back(bytes, static_cast<size_t>(length));
      }
      Tcl_ResetResult(interpreter);
      const bool merge_begin = arguments[0] == "iMerge" &&
                               arguments.size() == 2 &&
                               arguments[1] == "-begin";
      const bool record = self.trace_source_ && self.frames_.size() == 1 &&
                          self.merges_.empty() &&
                          (merge_begin || arguments[0] == "iCall" ||
                           arguments[0] == "iWrite" || arguments[0] == "iRead");
      SourceAction action;
      if (record) {
        action = {site ? site->source : std::numeric_limits<size_t>::max(),
                  site ? site->range.begin : 0,
                  self.operations_.size(),
                  0,
                  self.frames_.back().prefix,
                  {}};
        if (site) {
          for (const auto& word : site->command->words) {
            for (const auto& range : word.vector_ranges) {
              action.vector_brackets.push_back(range.begin);
              action.vector_brackets.push_back(range.end - 1);
            }
          }
        }
      }
      if (record) {
        action.arguments.assign(arguments.begin() + 1, arguments.end());
      }
      const auto finish_action = [&] {
        if (record) {
          action.end = self.operations_.size();
          if (merge_begin) self.main_merge_action_ = self.actions_.size();
          self.actions_.push_back(std::move(action));
        }
        if (self.main_merge_action_ && self.merges_.empty()) {
          self.actions_.at(*self.main_merge_action_).end =
              self.operations_.size();
          self.main_merge_action_.reset();
        }
      };
      try {
        self.Command(arguments, origin, site ? &*site : nullptr);
      } catch (...) {
        finish_action();
        throw;
      }
      finish_action();
      return TCL_OK;
    } catch (const std::exception& error) {
      const std::string message = origin + ": " + error.what();
      Tcl_SetObjResult(
          interpreter,
          Tcl_NewStringObj(message.data(), static_cast<int>(message.size())));
      return TCL_ERROR;
    }
  }

  using History = std::unordered_map<size_t, std::string>;

  size_t HistoryOffset(const BitReference& bit) const {
    const auto& node = netlist_->nodes.at(bit.node);
    return node.left >= node.right ? node.left - bit.index
                                   : bit.index - node.left;
  }

  void RememberHistory(const Operation& operation) {
    if (!netlist_ || operation.arguments.size() != 3) return;
    const auto& flag = operation.arguments[2];
    if (flag != "-LastReadValue" && flag != "-LastMiscompareValue") return;
    const auto target = ResolvePdlTarget(*netlist_, operation.arguments[0]);
    for (const auto& bit : target.bits) {
      ValidateStateTarget(*netlist_, bit.node);
    }
    if (!history_context_) history_context_ = std::make_unique<z3::context>();
    const auto value = PdlNumber(*history_context_, operation.arguments[1],
                                 target.bits.size());
    const bool read = flag == "-LastReadValue";
    auto& history = read ? read_history_ : miscompare_history_;
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      auto entry = history.find(bit.node);
      if (entry == history.end()) {
        entry =
            history
                .emplace(bit.node,
                         std::string(netlist_->nodes.at(bit.node).Width(), 'x'))
                .first;
      }
      char data = value[i];
      if (read && bit.inverted && data != 'x') data = data == '1' ? '0' : '1';
      entry->second.at(HistoryOffset(bit)) = data;
    }
  }

  void RememberCallbackState(const Operation& operation) {
    if (!has_callbacks_ || (operation.arguments.size() == 3 &&
                            operation.arguments[2] != "-LastWrittenValue")) {
      return;
    }
    const auto target = ResolvePdlTarget(*netlist_, operation.arguments.at(0));
    if (!history_context_) history_context_ = std::make_unique<z3::context>();
    const auto bits = PdlNumber(*history_context_, operation.arguments.at(1),
                                target.bits.size());
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      const auto& node = netlist_->nodes[bit.node];
      auto& history = node.write_callback ? callback_values_ : callback_held_;
      auto [entry, inserted] = history.try_emplace(bit.node, node.Width(), 'x');
      char value = bits[i];
      if (bit.inverted && value != 'x') value = value == '1' ? '0' : '1';
      entry->second[HistoryOffset(bit)] = value;
    }
  }

  void InvalidateCallbackRead(size_t id, const std::string& requested) {
    const auto& node = netlist_->nodes.at(id);
    for (auto* history : {&read_history_, &miscompare_history_}) {
      auto& value = history->try_emplace(id, node.Width(), 'x').first->second;
      for (size_t i = 0; i < requested.size(); ++i) {
        if (requested[i] == '1') value.at(i) = '?';
      }
    }
  }

  bool CanonicalRootScan(const Operation& scan, bool modeled_named = false) {
    if (!netlist_ || scan.arguments.empty()) return false;
    const auto found = netlist_->node_ids.find(scan.arguments[0]);
    if (found == netlist_->node_ids.end()) return false;
    const auto& root = netlist_->nodes[found->second];
    if (root.scope || !root.scan_interface ||
        root.scan_interface->kind != ScanInterfaceKind::kClient) {
      return false;
    }
    if (!history_context_) history_context_ = std::make_unique<z3::context>();
    try {
      const auto normalized = NormalizeScan(*history_context_, scan);
      const auto fields = ReadScanFields(normalized);
      if (modeled_named) {
        if (fields.chain.empty() || fields.instruction) return false;
        if (!metadata_scheduler_) {
          metadata_scheduler_ = std::make_unique<ScanScheduler>(*netlist_);
        }
        return metadata_scheduler_->NormalizeScan(scan).arguments ==
               scan.arguments;
      }
      if (!fields.chain.empty()) {
        if (!named_opaque_root_) {
          named_opaque_root_ = false;
          named_opaque_root_ =
              DescribeOpaqueScans(*netlist_).client == scan.arguments[0];
        }
        if (!*named_opaque_root_) return false;
      }
      return normalized.arguments == scan.arguments && !fields.instruction &&
             ScanChainName(scan.arguments[0], fields.chain) == fields.chain;
    } catch (const std::runtime_error&) {
      return false;
    }
  }

  bool RawOmissionNote(const Operation& note) {
    if (!netlist_ || !IsRawHistoryOmissionNote(note)) return false;
    const bool named = !note.arguments[1].compare(
        0, sizeof(kNamedRawHistoryOmission) - 1, kNamedRawHistoryOmission);
    const std::string_view prefix =
        named ? kNamedRawHistoryOmission : "modeled raw history omitted ";
    bool valid = false;
    const auto words =
        pdl::SplitList(note.arguments[1].substr(prefix.size()), &valid);
    if (!valid || words.empty() || words[0] != "iScan") return false;
    const Operation scan{"iScan", {words.begin() + 1, words.end()}, "", {}};
    return CanonicalRootScan(scan, named) &&
           RawHistoryOmissionText(scan, named) == note.arguments[1];
  }

  bool OpaqueStableNote(const Operation& note) const {
    constexpr std::string_view kPrefix = "modeled opaque child stable scan ";
    if (!netlist_ || note.arguments.size() != 2 ||
        (note.arguments[0] != "-comment" && note.arguments[0] != "-status") ||
        note.arguments[1].compare(0, kPrefix.size(), kPrefix)) {
      return false;
    }
    const auto text =
        std::string_view(note.arguments[1]).substr(kPrefix.size());
    const auto separator = text.find(' ');
    if (separator == std::string_view::npos) return false;
    const auto found =
        netlist_->node_ids.find(std::string(text.substr(0, separator)));
    if (found == netlist_->node_ids.end()) return false;
    const auto& child = netlist_->nodes[found->second];
    if (!child.scope || !child.scan_interface ||
        child.scan_interface->kind != ScanInterfaceKind::kClient) {
      return false;
    }
    const auto& load = child.scan_interface->default_load;
    const auto bits = text.substr(separator + 1);
    if (!load || load->is_string || !load->sized ||
        bits.size() != load->data.size() + 2 || bits.substr(0, 2) != "0b" ||
        bits.find_first_not_of("01x", 2) != std::string_view::npos) {
      return false;
    }
    return std::any_of(
        child.scan_interface->ports.begin(), child.scan_interface->ports.end(),
        [&](const auto& bit) {
          return bit.node < netlist_->nodes.size() &&
                 netlist_->nodes[bit.node].kind == "ScanOutPort" &&
                 !netlist_->nodes[bit.node].inputs.count("Source");
        });
  }

  std::optional<std::pair<size_t, std::string>> CallbackHistoryNote(
      const std::vector<std::string>& arguments) const {
    if (!netlist_) return std::nullopt;
    const bool tagged = arguments.size() == 2 && (arguments[0] == "-comment" ||
                                                  arguments[0] == "-status");
    if (!tagged && arguments.size() != 1) return std::nullopt;
    const auto& note = arguments[tagged ? 1 : 0];
    const std::string prefix = "modeled callback read history unavailable ";
    if (note.rfind(prefix, 0) != 0) return std::nullopt;
    const auto separator = note.find(' ', prefix.size());
    const auto requested =
        note.substr(prefix.size(), separator - prefix.size());
    const auto name =
        separator == std::string::npos ? "" : note.substr(separator + 1);
    const auto node = netlist_->node_ids.find(name);
    if (node == netlist_->node_ids.end() ||
        !netlist_->nodes[node->second].read_callback ||
        requested.size() != netlist_->nodes[node->second].Width() ||
        requested.find_first_not_of("01") != std::string::npos) {
      return std::nullopt;
    }
    return std::make_pair(node->second, requested);
  }
  void RestoreCallbackReadInvalidation(const Operation& operation) {
    if (const auto note = CallbackHistoryNote(operation.arguments)) {
      InvalidateCallbackRead(note->first, note->second);
    }
  }

  struct CallbackBinding {
    std::string instance;
    ProcedureKey procedure;
  };
  const CallbackBinding& BindCallback(size_t id, bool read) {
    const auto key = std::make_pair(id, read);
    const auto old = callback_bindings_.find(key);
    if (old != callback_bindings_.end()) return old->second;
    const auto& node = netlist_->nodes.at(id);
    const auto& descriptor = read ? *node.read_callback : *node.write_callback;
    auto owner = descriptor.name_space;
    if (owner.rfind("::", 0) == 0) owner.erase(0, 2);
    size_t scope = node.scope, matched = 0;
    std::optional<CallbackBinding> binding;
    while (true) {
      const auto& ancestor = netlist_->scopes.at(scope);
      const auto module = ancestor.definition.name();
      if ((owner == module || owner.rfind(module + "::", 0) == 0) &&
          (!binding || module.size() > matched)) {
        matched = module.size();
        binding = CallbackBinding{
            ancestor.path,
            {module, owner == module ? "" : owner.substr(module.size() + 2),
             descriptor.procedure}};
      }
      if (scope == 0) break;
      scope = ancestor.parent;
    }
    if (binding) {
      return callback_bindings_.emplace(key, std::move(*binding)).first->second;
    }
    throw std::runtime_error(
        "callback namespace must name an ancestor module: " +
        descriptor.name_space);
  }

  bool PendingCallback(size_t index) const {
    return operations_[index].callback &&
           !consumed_callback_operations_.count(index);
  }
  std::optional<BitReference> WriteCallbackSource(BitReference bit) {
    using Bit = std::pair<size_t, int64_t>;
    std::vector<std::pair<Bit, bool>> path;
    std::set<Bit> visiting;
    std::optional<BitReference> result;
    while (bit.node != std::numeric_limits<size_t>::max()) {
      const Bit key{bit.node, bit.index};
      const auto cached = callback_sources_.find(key);
      if (cached != callback_sources_.end()) {
        result = cached->second;
        if (result) result->inverted = result->inverted != bit.inverted;
        break;
      }
      if (!visiting.insert(key).second) break;
      if (callback_dependency_budget_ == 0) {
        throw std::runtime_error("callback dependency resource limit exceeded");
      }
      --callback_dependency_budget_;
      path.push_back({key, bit.inverted});
      const auto& node = netlist_->nodes.at(bit.node);
      if (node.write_callback) {
        result = bit;
        break;
      }
      const char* role = nullptr;
      if (node.kind == "DataRegister" &&
          !node.constants.count("AddressValue")) {
        role = "WriteDataSource";
      } else if ((node.model == ModelKind::kAlias ||
                  node.kind == "DataInPort" || node.kind == "DataOutPort") &&
                 !node.inputs.count("Enable") &&
                 !node.properties.count("EnableSource")) {
        role = "Source";
      }
      if (!role || !node.inputs.count(role)) break;
      auto next = node.inputs.at(role).at(HistoryOffset(bit));
      next.inverted = next.inverted != bit.inverted;
      bit = next;
    }
    for (const auto& [key, inverted] : path) {
      auto source = result;
      if (source) source->inverted = source->inverted != inverted;
      callback_sources_.emplace(key, source);
    }
    return result;
  }
  std::vector<Operation> CallbackOrdinaryAccesses(const Operation& operation,
                                                  const PdlTarget& target,
                                                  const std::string& bits,
                                                  bool deferred = false) {
    const bool read = operation.command == "iRead";
    std::map<std::pair<size_t, int64_t>, char> ordinary;
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      const auto& node = netlist_->nodes.at(bit.node);
      if (read ? node.read_callback.has_value()
               : node.write_callback.has_value()) {
        continue;
      }
      const bool automatic = !read && WriteCallbackSource(bit).has_value();
      if (automatic != deferred) continue;
      const char value = bit.inverted && bits[i] != 'x'
                             ? (bits[i] == '1' ? '0' : '1')
                             : bits[i];
      ordinary[{bit.node, bit.index}] = value;
    }
    std::vector<Operation> result;
    auto next = ordinary.begin();
    while (next != ordinary.end()) {
      const auto id = next->first.first;
      const auto& node = netlist_->nodes.at(id);
      auto left = next->first.second, right = left;
      std::string data(1, next->second);
      ++next;
      while (next != ordinary.end() && next->first.first == id &&
             next->first.second == right + 1) {
        right = next->first.second;
        data += next->second;
        ++next;
      }
      if (node.left >= node.right) {
        std::swap(left, right);
        std::reverse(data.begin(), data.end());
      }
      auto name = node.BaseName();
      if (node.indexed) {
        name += "[" + std::to_string(left);
        if (left != right) name += ":" + std::to_string(right);
        name += "]";
      }
      result.push_back({operation.command,
                        {std::move(name), "0b" + data},
                        operation.origin,
                        operation.call_stack,
                        false,
                        operation.callback_execution,
                        operation.callback_read_execution,
                        operation.callback_preserved,
                        operation.callback_prerequisite});
    }
    return result;
  }
  void CheckCallbackCallerWrite(const Operation& operation) {
    const bool state = operation.command == "iState" &&
                       (operation.arguments.size() != 3 ||
                        operation.arguments[2] == "-LastWrittenValue");
    if ((operation.command != "iWrite" && !state) || operation.callback ||
        std::all_of(callback_caller_writes_.begin(),
                    callback_caller_writes_.end(),
                    [](const auto& writes) { return writes.empty(); })) {
      return;
    }
    const auto target = ResolvePdlTarget(*netlist_, operation.arguments.at(0));
    const auto bits =
        state ? PdlNumber(*history_context_, operation.arguments.at(1),
                          target.bits.size())
              : PdlDataValue(*history_context_, *netlist_, target,
                             operation.arguments.at(1));
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      char value = bits[i];
      if (bit.inverted && value != 'x') value = value == '0' ? '1' : '0';
      for (const auto& writes : callback_caller_writes_) {
        const auto found = writes.find({bit.node, bit.index});
        if (found != writes.end() && found->second != value) {
          throw std::runtime_error(
              "callback endpoint conflicts with caller goal " +
              netlist_->nodes.at(bit.node).name + "[" +
              std::to_string(bit.index) + "]");
        }
      }
    }
  }

  bool ExecuteCallbacks(const Operation& apply) {
    if (!has_callbacks_ || !pending_) return false;
    const size_t begin = pending_operation_;
    const auto first = operations_.begin() + begin;
    bool selected = false;
    for (size_t index = begin; index < operations_.size(); ++index) {
      if (PendingCallback(index)) {
        selected = true;
        break;
      }
    }
    if (!selected) return false;
    const std::vector<Operation> pending(first, operations_.end());
    using Key = std::pair<size_t, bool>;
    using Order = std::pair<size_t, size_t>;
    struct Request {
      Key key;
      Order source;
      std::string data, requested;
    };
    QueuedAccesses queue;
    std::map<Key, size_t> positions;
    std::vector<Request> requests;
    std::vector<bool> mixed_reads(pending.size(), false);
    std::map<size_t, std::vector<size_t>> read_anchors;
    std::map<std::pair<size_t, int64_t>, Order> write_sources;
    std::vector<size_t> anchors;
    bool ordinary_pending = false;
    bool access_together = false, raw_access = false;
    for (size_t i = 0; i < pending.size(); ++i) {
      const auto& op = pending[i];
      if (op.callback && consumed_callback_operations_.count(begin + i)) {
        continue;
      }
      const bool read = op.command == "iRead";
      if (read || op.command == "iWrite") {
        const auto target = ResolvePdlTarget(*netlist_, op.arguments.at(0));
        access_together = access_together || !target.access_together.empty();
        ValidatePdlAccessTarget(*netlist_, target, read);
        const auto data = op.arguments.size() > 1
                              ? PdlDataValue(*history_context_, *netlist_,
                                             target, op.arguments[1])
                              : std::string(target.bits.size(), 'x');
        if (!read) {
          for (size_t operand = 0; operand < target.bits.size(); ++operand) {
            const auto& bit = target.bits[operand];
            write_sources[{bit.node, bit.index}] = {i, operand};
          }
        }
        if (!op.callback) {
          ordinary_pending = true;
          queue.Add(op, target, data);
          continue;
        }
        for (const auto& bit : target.bits) {
          const auto& node = netlist_->nodes.at(bit.node);
          if (!(read ? node.read_callback : node.write_callback)) {
            mixed_reads[i] = read;
            continue;
          }
          const Key key{bit.node, read};
          if (read) {
            auto& sources = read_anchors[bit.node];
            if (sources.empty() || sources.back() != begin + i) {
              sources.push_back(begin + i);
            }
          }
          if (positions.emplace(key, requests.size()).second) {
            requests.push_back({key,
                                {i, 0},
                                std::string(node.Width(), 'x'),
                                std::string(node.Width(), '0')});
          }
        }
        queue.Add(op, target, data);
        anchors.push_back(begin + i);
      } else if (op.command == "iScan") {
        raw_access = true;
        ordinary_pending = true;
      } else if (op.command == "iState" &&
                 (op.arguments.size() != 3 ||
                  op.arguments[2] == "-LastWrittenValue")) {
        const auto target = ResolvePdlTarget(*netlist_, op.arguments.at(0));
        queue.Assume(target, PdlNumber(*history_context_, op.arguments.at(1),
                                       target.bits.size()));
      }
    }
    using Bit = std::pair<size_t, int64_t>;
    std::map<Bit, char> caller_writes, effective_writes;
    std::map<Bit, std::pair<char, Order>> inferred;
    bool automatic = false;
    const auto writes = queue.WriteModel();
    std::set<size_t> loading;
    for (size_t i = 0; i < writes.first.size(); ++i) {
      const auto& bit = writes.first[i];
      const auto held = callback_held_.find(bit.node);
      if (writes.second[i] != 'x' &&
          (held == callback_held_.end() ||
           held->second[HistoryOffset(bit)] != writes.second[i])) {
        loading.insert(bit.node);
      }
    }
    for (bool read : {false, true}) {
      const auto [bits, values] = read ? queue.ReadModel() : queue.WriteModel();
      for (size_t i = 0; i < bits.size(); ++i) {
        const auto& bit = bits[i];
        const size_t offset = HistoryOffset(bit);
        if (!read) effective_writes[{bit.node, bit.index}] = values[i];
        const auto position = positions.find({bit.node, read});
        if (position == positions.end()) {
          const auto source =
              read ? std::optional<BitReference>{} : WriteCallbackSource(bit);
          if (source) {
            automatic = true;
            if (values[i] != 'x' && loading.count(bit.node)) {
              const char value =
                  source->inverted ? (values[i] == '1' ? '0' : '1') : values[i];
              const Bit key{source->node, source->index};
              const auto origin = write_sources.at({bit.node, bit.index});
              const auto [entry, inserted] =
                  inferred.emplace(key, std::make_pair(value, origin));
              if (!inserted && entry->second.first != value) {
                throw std::runtime_error(
                    "conflicting callback prerequisite requirements");
              }
              entry->second.second = std::min(entry->second.second, origin);
            }
          } else if (!read && values[i] != 'x') {
            caller_writes[{bit.node, bit.index}] = values[i];
          }
          continue;
        }
        auto& request = requests.at(position->second);
        request.data.at(offset) = values[i];
        request.requested.at(offset) = '1';
      }
    }
    std::set<Key> explicit_callbacks;
    if (automatic) {
      std::map<size_t, Order> effective_sources;
      for (const auto& [bit, origin] : write_sources) {
        const auto [entry, inserted] =
            effective_sources.emplace(bit.first, origin);
        if (!inserted) entry->second = std::min(entry->second, origin);
      }
      for (auto& request : requests) {
        explicit_callbacks.insert(request.key);
        if (!request.key.second) {
          request.source = effective_sources.at(request.key.first);
        }
      }
    }
    std::set<size_t> prerequisite_callbacks;
    for (const auto& [bit, inferred_value] : inferred) {
      const auto [id, index] = bit;
      prerequisite_callbacks.insert(id);
      const auto [value, source] = inferred_value;
      const Key key{id, false};
      if (positions.emplace(key, requests.size()).second) {
        requests.push_back({key, source,
                            std::string(netlist_->nodes[id].Width(), 'x'),
                            std::string(netlist_->nodes[id].Width(), '0')});
      }
      auto& request = requests.at(positions.at(key));
      request.source = std::min(request.source, source);
      const auto offset = HistoryOffset({id, index});
      if (request.data[offset] != 'x' && request.data[offset] != value) {
        throw std::runtime_error(
            "explicit write conflicts with callback prerequisite");
      }
      request.data[offset] = value;
      request.requested[offset] = '1';
    }
    std::vector<Operation> deferred;
    if (automatic) {
      requests.erase(
          std::remove_if(requests.begin(), requests.end(),
                         [&](const auto& request) {
                           if (explicit_callbacks.count(request.key)) {
                             return false;
                           }
                           const auto known =
                               callback_values_.find(request.key.first);
                           if (known == callback_values_.end()) return false;
                           for (size_t i = 0; i < request.data.size(); ++i) {
                             if (request.requested[i] == '1' &&
                                 request.data[i] != known->second[i]) {
                               return false;
                             }
                           }
                           prerequisite_callbacks.erase(request.key.first);
                           return true;
                         }),
          requests.end());
      std::stable_sort(requests.begin(), requests.end(),
                       [](const auto& left, const auto& right) {
                         return left.source < right.source;
                       });
      if (requests.empty()) {
        // Keep native alias/group semantics when no callback will run.
        for (const auto anchor : anchors) {
          auto operation = pending.at(anchor - begin);
          const auto target =
              ResolvePdlTarget(*netlist_, operation.arguments[0]);
          std::string data;
          for (const auto& bit : target.bits) {
            char value = effective_writes.at({bit.node, bit.index});
            if (bit.inverted && value != 'x') value = value == '1' ? '0' : '1';
            data += value;
          }
          operation.arguments[1] = "0b" + data;
          operation.callback = false;
          operations_.push_back(std::move(operation));
          consumed_callback_operations_.insert(anchor);
          if (trace_source_) {
            callback_action_ends_[anchor] = operations_.size() + 1;
          }
        }
        return false;
      }
      for (const auto anchor : anchors) {
        const auto& operation = pending.at(anchor - begin);
        if (operation.command != "iWrite") continue;
        const auto target =
            ResolvePdlTarget(*netlist_, operation.arguments.at(0));
        std::string data;
        for (const auto& bit : target.bits) {
          char value = effective_writes.at({bit.node, bit.index});
          if (bit.inverted && value != 'x') value = value == '1' ? '0' : '1';
          data += value;
        }
        auto accesses = CallbackOrdinaryAccesses(operation, target, data, true);
        for (auto& access : accesses) deferred.push_back(std::move(access));
      }
    }
    if (ValidatePdlApplyOptions(apply.arguments)) {
      throw std::runtime_error(
          "callback groups do not support iApply -together");
    }
    if (access_together) {
      throw std::runtime_error(
          "callback AccessTogether aliases are unsupported");
    }
    if (raw_access) {
      throw std::runtime_error(
          "mixed callback and raw accesses require separate Applies");
    }
    const size_t reset_generation = callback_reset_generation_;
    // Preflight all static bindings and loaded procedures before any effects.
    for (const auto& request : requests) {
      const auto& binding = BindCallback(request.key.first, request.key.second);
      if (!procedures_.count(binding.procedure)) {
        throw std::runtime_error("undefined iProc for callback " +
                                 netlist_->nodes[request.key.first].name);
      }
      const auto& definition = procedures_.at(binding.procedure);
      const auto& node = netlist_->nodes.at(request.key.first);
      const auto& descriptor =
          request.key.second ? *node.read_callback : *node.write_callback;
      if (descriptor.arguments.size() < definition.minimum_arguments ||
          descriptor.arguments.size() > definition.maximum_arguments) {
        throw std::runtime_error("wrong callback argument count for " +
                                 node.name);
      }
      if (std::find(callback_stack_.begin(), callback_stack_.end(),
                    request.key) != callback_stack_.end()) {
        throw std::runtime_error("callback cycle at " +
                                 netlist_->nodes[request.key.first].name);
      }
    }
    consumed_callback_operations_.insert(anchors.begin(), anchors.end());
    callback_caller_writes_.push_back(std::move(caller_writes));
    pending_ = ordinary_pending;
    try {
      for (const auto& request : requests) {
        const auto& source = pending.at(request.source.first);
        const auto [id, read] = request.key;
        const auto& node = netlist_->nodes.at(id);
        const auto& descriptor =
            read ? *node.read_callback : *node.write_callback;
        const auto& binding = BindCallback(id, read);
        auto relative = node.name;
        if (!binding.instance.empty()) {
          const auto prefix = binding.instance + ".";
          if (relative.rfind(prefix, 0) != 0) {
            throw std::runtime_error(
                "callback register is outside its namespace instance");
          }
          relative.erase(0, prefix.size());
        }
        std::vector<std::string> arguments;
        for (const auto& argument : descriptor.arguments) {
          if (argument.kind == CallbackArgumentKind::kData) {
            arguments.push_back("0b" + request.data);
          } else if (argument.kind == CallbackArgumentKind::kRegister) {
            arguments.push_back(relative);
          } else {
            arguments.push_back(argument.value.is_string
                                    ? argument.value.data
                                    : "0b" + argument.value.data);
          }
        }
        const auto definition = procedures_.at(binding.procedure);
        callback_stack_.push_back(request.key);
        if (read) ++read_callback_depth_;
        try {
          callbacks_executed_ = true;
          read_callbacks_executed_ = read_callbacks_executed_ || read;
          if (!read && prerequisite_callbacks.count(id)) {
            consumed_callback_operations_.insert(operations_.size());
            operations_.push_back({"iWrite",
                                   {node.name, "0b" + request.data},
                                   source.origin,
                                   source.call_stack,
                                   true,
                                   true,
                                   read_callback_depth_ != 0,
                                   source.callback_preserved,
                                   true});
          }
          if (read) {
            auto invocation = mixed_reads[request.source.first]
                                  ? Operation{"iRead",
                                              {node.name, "0b" + request.data},
                                              source.origin,
                                              source.call_stack,
                                              true,
                                              true,
                                              true,
                                              source.callback_preserved,
                                              source.callback_prerequisite}
                                  : source;
            invocation.callback_execution = true;
            invocation.callback_read_execution = true;
            invocation.callback_consumed = read_anchors.at(id);
            consumed_callback_operations_.insert(operations_.size());
            operations_.push_back(std::move(invocation));
          }
          InvokeProcedure(
              definition, binding.instance, arguments,
              std::string(read ? "ReadCallBack " : "WriteCallBack ") +
                  node.name + " at " + source.origin,
              source.origin, true);
        } catch (...) {
          if (read) --read_callback_depth_;
          callback_stack_.pop_back();
          throw;
        }
        if (read) --read_callback_depth_;
        callback_stack_.pop_back();
        if (read) {
          InvalidateCallbackRead(id, request.requested);
          // Standard PDL notes preserve unavailable bits without repeating IO.
          if (readback_) {
            operations_.push_back(
                {"iNote",
                 {"-comment", "modeled callback read history unavailable " +
                                  request.requested + " " + node.name},
                 source.origin,
                 source.call_stack,
                 false,
                 true,
                 true});
          }
          if (request.data.find_first_not_of('x') != std::string::npos) {
            zero_status_known_ = false;
          }
        } else {
          callback_values_[id] = request.data;
          operations_.push_back(
              {"iState",
               {node.name, "0b" + request.data, "-LastWrittenValue"},
               source.origin,
               source.call_stack,
               false,
               true,
               read_callback_depth_ != 0});
        }
      }
      if (reset_generation == callback_reset_generation_ && !deferred.empty()) {
        if (!pending_) pending_operation_ = operations_.size();
        pending_ = true;
        for (auto& operation : deferred) {
          operations_.push_back(std::move(operation));
        }
      }
      if (pending_) InvalidateReadHistory(apply);
    } catch (...) {
      callback_caller_writes_.pop_back();
      callback_failed_ = true;
      pending_ = true;
      pending_operation_ = begin;
      if (trace_source_) {
        for (size_t anchor : anchors) {
          callback_action_ends_[anchor] = operations_.size();
        }
      }
      throw;
    }
    callback_caller_writes_.pop_back();
    if (trace_source_) {
      for (size_t anchor : anchors) {
        callback_action_ends_[anchor] = operations_.size() + 1;
      }
    }
    pending_ = false;
    return true;
  }

  std::string ScanChainName(const std::string& name, const std::string& chain) {
    if (!netlist_) return chain;
    if (!scan_clients_loaded_) {
      for (const auto& node : netlist_->nodes) {
        if (!node.scan_interface ||
            (node.scan_interface->kind != ScanInterfaceKind::kClient &&
             node.scan_interface->kind != ScanInterfaceKind::kClientTap)) {
          continue;
        }
        if (node.scan_interface->kind == ScanInterfaceKind::kClientTap) {
          tap_clients_.insert(node.name);
        }
        auto& chains = scan_clients_[node.name];
        for (const auto& item : node.scan_interface->chains) {
          chains.insert(item.name);
        }
      }
      if (!netlist_->node_ids.count("unnamed") && netlist_->taps.size() == 1) {
        const bool declared = std::any_of(
            netlist_->nodes.begin(), netlist_->nodes.end(),
            [](const auto& node) {
              return node.scope == 0 && node.scan_interface.has_value();
            });
        const auto interfaces = EffectiveScanInterfaces(*netlist_);
        if (!declared && std::any_of(interfaces.begin(), interfaces.end(),
                                     [](const auto& binding) {
                                       return binding.scope == 0 &&
                                              binding.interface.kind ==
                                                  ScanInterfaceKind::kClientTap;
                                     })) {
          scan_clients_["unnamed"];
          tap_clients_.insert("unnamed");
        }
      }
      scan_clients_loaded_ = true;
    }
    const auto found = scan_clients_.find(name);
    if (found == scan_clients_.end()) {
      throw std::runtime_error("raw scan target must be a client interface");
    }
    if (found->second.empty() && (chain.empty() || chain == "default")) {
      return "";
    }
    if (chain.empty() || !found->second.count(chain)) {
      throw std::runtime_error("raw scan requires a valid client chain");
    }
    return chain;
  }

  void InvalidateReadHistory(const Operation& apply) {
    modeled_history_root_.reset();
    const bool together = ValidatePdlApplyOptions(apply.arguments);
    if (!pending_) return;
    const auto first = operations_.begin() + pending_operation_;
    if (std::none_of(first, operations_.end(), [](const auto& operation) {
          return operation.command == "iRead" || operation.command == "iScan";
        })) {
      return;
    }
    if (!history_context_) history_context_ = std::make_unique<z3::context>();
    bool zero_status_known = zero_status_known_;
    bool accesses = false, fallback_read = false;
    std::unordered_map<size_t, std::string> read_care;
    struct RawCare {
      size_t width = 0;
      bool compared = false, omitted = false;
    };
    std::map<std::pair<bool, std::pair<std::string, std::string>>, RawCare>
        raw_care;
    size_t raw_bits = 0;
    std::optional<size_t> root_source;
    std::optional<bool> raw_mode;
    bool raw_scan_applied = false;
    // Validate discarded operands too; commit history only after the whole
    // queue.
    for (auto it = first; it != operations_.end(); ++it) {
      const auto& operation = *it;
      if (operation.callback) continue;
      if (operation.command == "iScan") {
        const auto scan = NormalizeScan(*history_context_, operation);
        const auto fields = ReadScanFields(scan);
        const auto& name = scan.arguments[fields.target];
        const auto chain = ScanChainName(name, fields.chain);
        if (netlist_ && !fields.instruction && chain.empty()) {
          const auto root = netlist_->node_ids.find(name);
          if (root != netlist_->node_ids.end() &&
              !netlist_->nodes[root->second].scope &&
              netlist_->nodes[root->second].scan_interface &&
              netlist_->nodes[root->second].scan_interface->kind ==
                  ScanInterfaceKind::kClient) {
            root_source =
                fields.length == 1 && scan.arguments == operation.arguments
                    ? std::optional<size_t>(it - operations_.begin())
                    : std::nullopt;
          }
        }
        if (fields.instruction && netlist_ &&
            (netlist_->taps.empty() || !tap_clients_.count(name))) {
          throw std::runtime_error(
              "raw IR scan target must be a client TAP interface");
        }
        if (raw_mode && *raw_mode != fields.instruction) {
          throw std::runtime_error(
              "mixed raw IR and data scans are unsupported");
        }
        raw_mode = fields.instruction;
        const auto key =
            std::make_pair(fields.instruction, std::make_pair(name, chain));
        auto& latest = raw_care[key];
        const auto& expected = scan.arguments[fields.expected];
        const size_t width = expected.size() - 2;
        raw_bits = raw_bits - latest.width + width;
        if (raw_bits > 1024 * 1024) {
          throw std::runtime_error("raw scan queued payload resource limit");
        }
        latest = {width,
                  expected.find_first_not_of('x', 2) != std::string::npos,
                  operation.raw_history_omitted};
        raw_scan_applied = raw_scan_applied || !fields.instruction;
        continue;
      }
      const bool read = operation.command == "iRead";
      const bool write = operation.command == "iWrite";
      const bool state = operation.command == "iState";
      const bool controlling =
          state && (operation.arguments.size() != 3 ||
                    operation.arguments[2] == "-LastWrittenValue");
      if (!read && !write && !controlling) continue;
      if (read || write) accesses = true;
      if (!netlist_) {
        if (read) {
          fallback_read = true;
          if (operation.arguments.size() > 1) zero_status_known = false;
        } else if (controlling && fallback_read) {
          zero_status_known = false;
        }
        continue;
      }
      const auto target = ResolvePdlTarget(*netlist_, operation.arguments[0]);
      std::string bits;
      if (controlling) {
        bits = PdlNumber(*history_context_, operation.arguments[1],
                         target.bits.size());
        for (const auto& bit : target.bits) {
          ValidateStateAssumption(*netlist_, bit.node);
        }
      } else {
        ValidatePdlAccessTarget(*netlist_, target, read);
        bits = operation.arguments.size() == 2
                   ? PdlDataValue(*history_context_, *netlist_, target,
                                  operation.arguments[1])
                   : std::string(target.bits.size(), 'x');
      }
      if (!read && !controlling) continue;
      for (size_t i = 0; i < target.bits.size(); ++i) {
        const auto& bit = target.bits[i];
        auto entry = read_care.find(bit.node);
        if (read && entry == read_care.end()) {
          entry =
              read_care
                  .emplace(bit.node,
                           std::string(netlist_->nodes[bit.node].Width(), '?'))
                  .first;
        }
        if (entry == read_care.end()) continue;
        auto& care = entry->second.at(HistoryOffset(bit));
        if (read || care != '?') care = bits[i] == 'x' ? 'x' : '1';
      }
    }
    if (together && accesses && !raw_care.empty()) {
      throw std::runtime_error(
          "mixed raw preparation cannot use iApply -together");
    }
    if (root_source && raw_care.size() == 1) {
      modeled_history_root_ = root_source;
      operations_[*root_source].modeled_history_source = root_source;
    }
    for (const auto& [key, scan] : raw_care) {
      if (scan.compared) zero_status_known = false;
    }
    for (const auto& [key, scan] : raw_care) {
      if (scan.omitted) continue;
      raw_read_history_[key.second] = std::string(scan.width, '?');
      raw_miscompare_history_[key.second] = std::string(scan.width, '?');
    }
    opaque_history_scan_ = opaque_history_scan_ || raw_scan_applied;
    for (const auto& [id, care] : read_care) {
      if (care.find('1') != std::string::npos) zero_status_known = false;
      for (auto* history : {&read_history_, &miscompare_history_}) {
        auto& value = history->try_emplace(id, care.size(), 'x').first->second;
        for (size_t i = 0; i < care.size(); ++i) {
          if (care[i] != '?') value[i] = '?';
        }
      }
    }
    zero_status_known_ = zero_status_known;
  }

  const ReadbackUpdates* UpdateReadback(bool clear_status) {
    if (!readback_) return nullptr;
    const auto* updates = readback_(operations_, clear_status);
    if (!updates) return nullptr;
    for (const auto& update : updates->bits) {
      if (!netlist_ || update.node >= netlist_->nodes.size()) {
        throw std::runtime_error("invalid modeled readback node");
      }
      const auto& node = netlist_->nodes[update.node];
      if (update.index < std::min(node.left, node.right) ||
          update.index > std::max(node.left, node.right)) {
        throw std::runtime_error("invalid modeled readback bit index");
      }
      for (char value : {update.read, update.miscompare}) {
        if (value != '0' && value != '1' && value != 'x' && value != '?') {
          throw std::runtime_error("invalid modeled readback value");
        }
      }
    }
    std::vector<std::pair<std::string, std::string>> scan_keys;
    std::set<std::pair<std::string, std::string>> seen;
    for (const auto& update : updates->scans) {
      const auto key = std::make_pair(
          update.target, ScanChainName(update.target, update.chain));
      const auto applied = raw_read_history_.find(key);
      const auto pending = opaque_history_pending_.find(key);
      const size_t width =
          applied != raw_read_history_.end()
              ? applied->second.size()
              : (pending != opaque_history_pending_.end() ? pending->second
                                                          : 0);
      if (!netlist_ || !width || !seen.insert(key).second ||
          update.read.size() != width || update.miscompare.size() != width ||
          update.read.find_first_not_of("01x") != std::string::npos ||
          update.miscompare.find_first_not_of("01x") != std::string::npos) {
        throw std::runtime_error("invalid modeled raw readback delta");
      }
      scan_keys.push_back(key);
    }
    for (const auto& update : updates->bits) {
      const auto merge = [&](History& history, char value) {
        if (value == '?') return;
        auto entry = history.find(update.node);
        if (entry == history.end()) {
          entry = history
                      .emplace(update.node,
                               std::string(netlist_->nodes[update.node].Width(),
                                           'x'))
                      .first;
        }
        entry->second.at(HistoryOffset({update.node, update.index})) = value;
      };
      merge(read_history_, update.read);
      merge(miscompare_history_, update.miscompare);
    }
    for (size_t i = 0; i < updates->scans.size(); ++i) {
      raw_read_history_[scan_keys[i]] = updates->scans[i].read;
      raw_miscompare_history_[scan_keys[i]] = updates->scans[i].miscompare;
    }
    opaque_history_pending_.clear();
    zero_status_known_ =
        updates->failures && (clear_status || *updates->failures == 0);
    return updates;
  }

  std::string HistoryValue(const std::string& name, const std::string& target,
                           const std::string& format,
                           const std::string& chain = "") {
    const auto unavailable = [&] {
      throw std::runtime_error(
          name +
          " requires hardware readback; unavailable during static generation");
    };
    if (name == "iGetStatus") {
      if (zero_status_known_) return "0";
      if (const auto* updates = UpdateReadback(target == "-clear")) {
        if (updates->failures) return std::to_string(*updates->failures);
        throw std::runtime_error("modeled readback status is unknown");
      }
      unavailable();
    }
    if (!netlist_) unavailable();
    const auto node = netlist_->node_ids.find(target);
    const bool client = node != netlist_->node_ids.end() &&
                        netlist_->nodes[node->second].scan_interface &&
                        (netlist_->nodes[node->second].scan_interface->kind ==
                             ScanInterfaceKind::kClient ||
                         netlist_->nodes[node->second].scan_interface->kind ==
                             ScanInterfaceKind::kClientTap);
    if (client || (target == "unnamed" && node == netlist_->node_ids.end() &&
                   netlist_->taps.size() == 1)) {
      const auto key = std::make_pair(target, ScanChainName(target, chain));
      const auto& history =
          name == "iGetReadData" ? raw_read_history_ : raw_miscompare_history_;
      if (opaque_history_pending_.count(key)) {
        if (!readback_) unavailable();
        UpdateReadback(false);
      }
      auto entry = history.find(key);
      if (entry == history.end()) {
        throw std::runtime_error(
            "raw readback requires a preceding applied iScan");
      }
      if (entry->second.find('?') != std::string::npos) {
        UpdateReadback(false);
        entry = history.find(key);
      }
      if (entry->second.find('?') != std::string::npos) unavailable();
      return FormatHistory(entry->second, format);
    }
    if (!chain.empty()) {
      throw std::runtime_error(
          "hardware readback chain requires a client interface");
    }
    const auto resolved = ResolvePdlTarget(*netlist_, target);
    ValidatePdlAccessTarget(*netlist_, resolved, true);
    const bool read = name == "iGetReadData";
    const auto& history = read ? read_history_ : miscompare_history_;
    if (std::any_of(resolved.bits.begin(), resolved.bits.end(),
                    [&](const auto& bit) {
                      const auto entry = history.find(bit.node);
                      return entry != history.end() &&
                             entry->second.at(HistoryOffset(bit)) == '?';
                    })) {
      UpdateReadback(false);
    }
    std::string bits;
    bits.reserve(resolved.bits.size());
    for (const auto& bit : resolved.bits) {
      const auto entry = history.find(bit.node);
      char data =
          entry == history.end() ? 'x' : entry->second.at(HistoryOffset(bit));
      if (data == '?') unavailable();
      if (read && bit.inverted && data != 'x') data = data == '1' ? '0' : '1';
      bits += data;
    }
    return FormatHistory(bits, format);
  }

  std::string FormatHistory(const std::string& bits,
                            const std::string& format) {
    if (format == "-dec") {
      if (bits.find('x') != std::string::npos) return "X";
      if (Tcl_ExprString(interpreter_.get(), ("0b" + bits).c_str()) != TCL_OK) {
        throw std::runtime_error(Tcl_GetStringResult(interpreter_.get()));
      }
      return Tcl_GetStringResult(interpreter_.get());
    }
    if (format != "-hex") return "0b" + bits;
    std::string value = "0x";
    size_t width = (bits.size() - 1) % 4 + 1;
    for (size_t i = 0; i < bits.size(); width = 4) {
      unsigned digit = 0;
      size_t unknown = 0;
      for (size_t j = 0; j < width; ++j) {
        const char bit = bits[i++];
        unknown += bit == 'x';
        digit = digit * 2 + (bit == '1');
      }
      value += unknown == width ? 'X'
               : unknown        ? 'x'
                                : "0123456789abcdef"[digit];
    }
    return value;
  }

  void EvalWords(const std::vector<std::string>& words) {
    std::vector<Tcl_Obj*> objects;
    for (const auto& word : words) {
      auto* object =
          Tcl_NewStringObj(word.data(), static_cast<int>(word.size()));
      Tcl_IncrRefCount(object);
      objects.push_back(object);
    }
    internal_command_ = true;
    const int result =
        Tcl_EvalObjv(interpreter_.get(), static_cast<int>(objects.size()),
                     objects.data(), TCL_EVAL_DIRECT);
    for (auto* object : objects) Tcl_DecrRefCount(object);
    if (result != TCL_OK) {
      throw std::runtime_error(Tcl_GetStringResult(interpreter_.get()));
    }
  }

  static int Trace(void* data, Tcl_Interp* interpreter, int, const char*,
                   Tcl_Command, int count, Tcl_Obj* const values[]) {
    auto& self = *static_cast<Impl*>(data);
    if (self.internal_command_) {
      self.internal_command_ = false;
      return TCL_OK;
    }
    if (!count || (self.frames_.size() == 1 ? self.level_
                                            : self.frames_.back().level) != 0) {
      return TCL_OK;
    }
    try {
      const auto name = CommandName(Tcl_GetString(values[0]));
      if (name == "icl_pattern_dispatch" || name == "icl_pattern_scope" ||
          pdl::FindCommandSpec(name)) {
        return TCL_OK;
      }
    } catch (...) {
      Tcl_SetObjResult(interpreter,
                       Tcl_NewStringObj("PDL command validation failed", -1));
      return TCL_ERROR;
    }
    Tcl_SetObjResult(
        interpreter,
        Tcl_NewStringObj("command is not permitted in PDL level-0", -1));
    return TCL_ERROR;
  }

  void Validate(const std::vector<std::string>& words, const Site* site) const {
    // Reuse frontend option/value validation after Tcl substitutions resolve.
    if (words[0] == "iMerge") {
      if (words[1] != "-begin" && words[1] != "-end") {
        throw std::runtime_error("invalid iMerge option");
      }
      return;
    }
    std::vector<std::string> literal = words;
    if (words[0] == "iProc" && site && site->command->words.size() == 4) {
      const auto& body = site->command->words[3];
      if (body.body) {
        literal[3] = site->document->source().substr(
            body.content.begin, body.content.end - body.content.begin);
      }
      if (level_ == 0) {
        const pdl::Document validated(
            site->document->source().substr(
                site->range.begin, site->range.end - site->range.begin),
            false, pdl::ParseContext{0, binding_, definition_namespace_});
        if (!validated.ok()) {
          throw std::runtime_error(validated.error_message());
        }
      }
    }
    Tcl_DString list;
    Tcl_DStringInit(&list);
    for (const auto& word : literal) {
      Tcl_DStringAppendElement(&list, word.c_str());
    }
    const std::string source(Tcl_DStringValue(&list));
    Tcl_DStringFree(&list);
    const pdl::Document validated(
        source, false, pdl::ParseContext{1, binding_, definition_namespace_});
    if (!validated.ok()) throw std::runtime_error(validated.error_message());
  }

  void ExecuteMergeCalls(size_t index) {
    const auto calls = std::move(merges_.at(index).calls);
    for (const auto& call : calls) {
      InvokeProcedure(call.definition, call.instance, call.arguments,
                      call.invocation, call.origin);
    }
  }

  void InvokeProcedure(const Procedure& definition, const std::string& instance,
                       const std::vector<std::string>& arguments,
                       const std::string& invocation, const std::string& origin,
                       bool apply_tail = false) {
    if (frames_.size() >= kRecursionLimit) {
      throw std::runtime_error("PDL procedure recursion limit exceeded");
    }
    std::vector<std::string> call{definition.tcl_name};
    if (!definition.parameters.empty()) {
      const auto scope = instance_scopes_.find(instance);
      if (scope == instance_scopes_.end()) {
        throw std::runtime_error(
            "ICL parameters require an elaborated instance " + instance);
      }
      for (const auto& name : definition.parameters) {
        const auto& value = scope->second->parameters.at(name);
        if (value.is_string) {
          call.push_back(value.data);
        } else if (value.data.find('x') != std::string::npos) {
          call.push_back("0b" + value.data);
        } else {
          EvalWords({"expr", "0b" + value.data});
          call.push_back(Tcl_GetStringResult(interpreter_.get()));
        }
      }
    }
    if (!definition.parameters.empty()) call.emplace_back();
    call.insert(call.end(), arguments.begin(), arguments.end());
    frames_.push_back({instance, "", "", invocation, 0, definition.level});
    const size_t merge_count = merges_.size();
    try {
      EvalWords(call);
      if (merges_.size() != merge_count) {
        throw std::runtime_error("iProc returned with unclosed iMerge");
      }
      if (apply_tail && pending_) Command({"iApply"}, origin, nullptr);
      if (apply_tail && callback_failed_) {
        throw std::runtime_error("failed callback queue requires iReset");
      }
      frames_.pop_back();
    } catch (const std::exception& error) {
      if (!callback_stack_.empty()) {
        for (size_t i = merges_.size(); i > merge_count; --i) {
          try {
            ExecuteMergeCalls(i - 1);
          } catch (const std::exception&) {
            // Retain completed calls while propagating the original failure.
          }
          std::vector<std::string> stack;
          for (size_t frame = 0; frame < merges_[i - 1].depth; ++frame) {
            if (!frames_[frame].invocation.empty()) {
              stack.push_back(frames_[frame].invocation);
            }
          }
          operations_.push_back({"iMerge",
                                 {"-end"},
                                 merges_[i - 1].origin,
                                 std::move(stack),
                                 false,
                                 true,
                                 read_callback_depth_ != 0});
        }
      }
      merges_.resize(merge_count);
      frames_.pop_back();
      throw std::runtime_error(std::string(error.what()) + "\n  while " +
                               invocation);
    }
    if (!merges_.empty() && merges_.back().depth == frames_.size() &&
        pending_) {
      throw std::runtime_error(
          "merged iCall returned with unapplied operations");
    }
  }

  const ModuleDefinition& InstanceModule(const std::string& path) const {
    if (!instance_scopes_.empty()) {
      const auto found = instance_scopes_.find(path);
      if (found == instance_scopes_.end()) {
        throw std::runtime_error("unknown instance " + path);
      }
      return found->second->definition;
    }
    const ModuleDefinition* module = &design_.modules().at(top_);
    size_t begin = 0;
    while (begin < path.size()) {
      const auto dot = path.find('.', begin);
      const auto name =
          path.substr(begin, dot == std::string::npos ? dot : dot - begin);
      const auto& members = module->document->members();
      const auto scope =
          members.find(static_cast<int>(module->declaration_index));
      if (scope == members.end()) {
        throw std::runtime_error("unknown instance " + path);
      }
      const auto found = scope->second.find(name);
      if (found == scope->second.end() || found->second.size() != 1) {
        throw std::runtime_error("unknown or ambiguous instance " + path);
      }
      module = &design_.ResolveInstance(*module, found->second.front());
      if (dot == std::string::npos) break;
      begin = dot + 1;
      if (begin == path.size()) {
        throw std::runtime_error("invalid instance path " + path);
      }
    }
    return *module;
  }

  void RememberStatusClear(const std::string& origin,
                           const std::string& count) {
    Operation operation{"iGetStatus", {"-clear", count}, origin, {}};
    operation.callback_execution = !callback_stack_.empty();
    operation.callback_read_execution = read_callback_depth_ != 0;
    for (const auto& frame : frames_) {
      if (!frame.invocation.empty()) {
        operation.call_stack.push_back(frame.invocation);
      }
    }
    operations_.push_back(std::move(operation));
  }

  void Command(const std::vector<std::string>& words, const std::string& origin,
               const Site* site) {
    if (++executed_ > 1000000) {
      throw std::runtime_error("PDL operation limit exceeded");
    }
    const auto name = CommandName(words.at(0));
    const auto* spec = pdl::FindCommandSpec(name);
    if (!spec) throw std::runtime_error("unsupported PDL command " + name);
    const auto count = words.size() - 1;
    if (count < static_cast<size_t>(spec->minimum) ||
        count > static_cast<size_t>(spec->maximum)) {
      throw std::runtime_error("wrong number of arguments for " + name);
    }
    if ((name == "iWrite" || name == "iRead" || name == "iState") &&
        words[1].empty()) {
      throw std::runtime_error("empty target for " + name);
    }
    Validate(words, site);
    if (name == "iReset" && !merges_.empty()) {
      throw std::runtime_error("iReset is forbidden within iMerge calls");
    }
    if (name == "iReset" && !frames_.back().instance.empty()) {
      throw std::runtime_error("cannot retarget iReset from a child module");
    }
    ++frames_.back().command;
    const auto prefix = [&] {
      return JoinPath(frames_.back().instance, frames_.back().prefix);
    };
    if (!merges_.empty() && merges_.back().depth == frames_.size() &&
        name != "iCall" && name != "iNote" && name != "iMerge") {
      throw std::runtime_error("only iCall and iNote are allowed in iMerge");
    }
    if (name == "iPDLLevel") {
      if ((words[1] != "0" && words[1] != "1") ||
          (words.size() != 2 && (words.size() != 4 || words[2] != "-version" ||
                                 words[3] != "STD_1687_2014"))) {
        throw std::runtime_error(
            "expected iPDLLevel 0|1 -version STD_1687_2014");
      }
      level_ = words[1] == "0" ? 0 : 1;
      return;
    }
    if (name == "iProcsForModule") {
      binding_ = CommandName(words[1]);
      if (!design_.modules().count(binding_) &&
          !module_parameters_.count(binding_)) {
        throw std::runtime_error("unknown module " + binding_);
      }
      definition_namespace_ = "";
      if (words.size() != 2) {
        if (words.size() != 4 || words[2] != "-iProcNameSpace") {
          throw std::runtime_error("expected -iProcNameSpace namespace");
        }
        definition_namespace_ = words[3];
      }
      return;
    }
    if (name == "iProc") {
      if (binding_.empty()) {
        throw std::runtime_error("iProc requires iProcsForModule");
      }
      const auto key = ProcedureKey{binding_, definition_namespace_, words[1]};
      const auto old = procedures_.find(key);
      const std::string tcl_name =
          "::icl_pattern_proc_" + std::to_string(next_procedure_++);
      const auto body = std::make_shared<pdl::Document>(
          words[3], false,
          pdl::ParseContext{level_, binding_, definition_namespace_, false});
      if (!body->ok()) throw std::runtime_error(body->error_message());
      std::vector<std::string> parameters;
      const auto module_parameters = module_parameters_.find(binding_);
      if (module_parameters != module_parameters_.end()) {
        parameters.assign(module_parameters->second.begin(),
                          module_parameters->second.end());
      } else {
        for (const auto* node : design_.modules().at(binding_).ast->body()) {
          const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
          if (statement &&
              (statement->named_kind() == icl::NamedKind::kParameter ||
               statement->named_kind() == icl::NamedKind::kLocalParameter)) {
            parameters.push_back(statement->name());
          }
        }
      }
      bool valid = false;
      const auto formals = pdl::SplitList(words[2], &valid);
      if (!valid) throw std::runtime_error("invalid iProc argument list");
      size_t minimum_arguments = 0, maximum_arguments = formals.size();
      for (size_t index = 0; index < formals.size(); ++index) {
        const auto& formal = formals[index];
        const auto fields = pdl::SplitList(formal, &valid);
        if (!valid || fields.empty()) {
          throw std::runtime_error("invalid iProc argument");
        }
        if (index + 1 == formals.size() && fields[0] == "args") {
          maximum_arguments = std::numeric_limits<size_t>::max();
        } else if (fields.size() == 1) {
          minimum_arguments = index + 1;
        }
        if (std::find(parameters.begin(), parameters.end(), fields[0]) !=
            parameters.end()) {
          throw std::runtime_error(
              "iProc argument conflicts with ICL parameter " + fields[0]);
        }
      }
      std::string signature;
      for (const auto& parameter : parameters) signature += parameter + " ";
      // An ICL parameter named args must not become Tcl's variadic final
      // formal.
      if (!parameters.empty()) signature += "__icl_pattern_guard ";
      signature += words[2];
      EvalWords({"proc", tcl_name, signature,
                 Instrument(origin + " (evaluated iProc body)", body)});
      if (old != procedures_.end()) {
        warnings_.push_back(origin + ": warning: iProc " + binding_ + "/" +
                            definition_namespace_ + "::" + words[1] +
                            " overrides " + old->second.origin);
      }
      procedures_.insert_or_assign(
          key, Procedure{tcl_name, origin, level_, std::move(parameters),
                         minimum_arguments, maximum_arguments});
      return;
    }
    if (name == "iPrefix") {
      const auto next = words[1] == "-" ? "" : words[1];
      InstanceModule(JoinPath(frames_.back().instance, next));
      frames_.back().prefix = next;
      return;
    }
    if (name == "iUseProcNameSpace") {
      frames_.back().procedure_namespace = words[1] == "-" ? "" : words[1];
      return;
    }
    if (name == "iCall") {
      if (!merges_.empty() && merges_.back().depth == frames_.size() &&
          pending_) {
        throw std::runtime_error(
            "merged iCall has unapplied operations at entry");
      }
      const auto dot = words[1].rfind('.');
      std::string instance = prefix();
      std::string procedure = words[1];
      if (dot != std::string::npos) {
        instance = JoinPath(instance, procedure.substr(0, dot));
        procedure.erase(0, dot + 1);
      }
      auto ns = frames_.back().procedure_namespace;
      const auto separator = procedure.rfind("::");
      if (separator != std::string::npos) {
        ns = procedure.substr(0, separator);
        procedure.erase(0, separator + 2);
      }
      const auto& module = InstanceModule(instance);
      const auto found = procedures_.find({module.name(), ns, procedure});
      if (found == procedures_.end()) {
        throw std::runtime_error("undefined iProc " + module.name() + "/" + ns +
                                 "::" + procedure);
      }
      const auto definition = found->second;
      const auto invocation = "iCall " + words[1] + " at " + origin +
                              " (command " +
                              std::to_string(frames_.back().command) + ")";
      if (!merges_.empty() && merges_.back().depth == frames_.size() &&
          merges_.back().defer_calls) {
        merges_.back().calls.push_back({definition,
                                        instance,
                                        {words.begin() + 2, words.end()},
                                        invocation,
                                        origin});
      } else {
        InvokeProcedure(definition, instance, {words.begin() + 2, words.end()},
                        invocation, origin);
      }
      return;
    }
    if (name == "iGetReadData" || name == "iGetMiscompares" ||
        name == "iGetStatus") {
      std::string chain, format;
      for (size_t i = 2; i < words.size(); ++i) {
        if (words[i] == "-chain") {
          chain = words.at(++i);
        } else {
          format = words[i];
        }
      }
      const auto value = HistoryValue(
          name,
          words.size() > 1
              ? (name == "iGetStatus" ? words[1] : JoinPath(prefix(), words[1]))
              : "",
          format, chain);
      if (name == "iGetStatus" && words.size() == 2 && words[1] == "-clear") {
        RememberStatusClear(origin, value);
      }
      Tcl_SetObjResult(
          interpreter_.get(),
          Tcl_NewStringObj(value.data(), static_cast<int>(value.size())));
      return;
    }
    Operation operation{name, {words.begin() + 1, words.end()}, origin, {}};
    if (name == "iNote" && netlist_) {
      RestoreCallbackReadInvalidation(operation);
      if (const auto note = ReadModeledChildHistoryNote(*netlist_, operation);
          note && modeled_history_root_ && !operations_.empty() &&
          netlist_->nodes[note->root].name ==
              operations_[*modeled_history_root_].arguments[0] &&
          (operations_.back().command == "iApply" ||
           operations_.back().modeled_history_source ==
               modeled_history_root_)) {
        operation.modeled_history_source = modeled_history_root_;
        try {
          if (history_readback_ && history_readback_(operations_, operation)) {
            operation.modeled_raw_history = true;
            opaque_history_pending_[{netlist_->nodes[note->client].name, ""}] =
                note->expected.size() - 2;
          }
        } catch (const std::runtime_error&) {
          // Informational metadata cannot supply an unavailable observation.
        }
      }
      if (const auto note = ReadOpaqueChildHistoryNote(*netlist_, operation);
          note && opaque_history_scan_) {
        const size_t width = note->expected.size() - 2;
        opaque_history_pending_[{netlist_->nodes[note->client].name, ""}] =
            width;
      }
    }
    operation.callback_execution = !callback_stack_.empty();
    operation.callback_read_execution = read_callback_depth_ != 0;
    if (name == "iWrite" && operation.arguments.size() > 2) {
      // Every mode permits a load; retain the native explicit-load policy.
      operation.arguments.resize(2);
    }
    for (const auto& frame : frames_) {
      if (!frame.invocation.empty()) {
        operation.call_stack.push_back(frame.invocation);
      }
    }
    if (const auto expected = ReadStatusClearNote(operation);
        expected &&
        (merges_.empty() || merges_.back().depth != frames_.size())) {
      try {
        const auto actual =
            zero_status_known_
                ? std::optional<uint64_t>{0}
                : (status_readback_ ? status_readback_(operations_)
                                    : std::nullopt);
        if (actual == expected) {
          const auto value = HistoryValue("iGetStatus", "-clear", "");
          RememberStatusClear(origin, value);
          return;
        }
      } catch (const std::runtime_error&) {
        // Informational notes cannot supply an unavailable or unknown count.
      }
    }
    if (name == "iTake") {
      if (operation.arguments.empty() ||
          operation.arguments[0] == "-persistent") {
        operation.arguments.insert(operation.arguments.begin(), prefix());
      } else {
        operation.arguments[0] = JoinPath(prefix(), operation.arguments[0]);
      }
    }
    if (name == "iWrite" || name == "iRead" || name == "iState" ||
        name == "iRelease" || name == "iClock" || name == "iClockOverride") {
      if (!operation.arguments.empty() && operation.arguments[0].empty()) {
        throw std::runtime_error("empty target for " + name);
      }
      if (!operation.arguments.empty() && operation.arguments[0][0] != '-') {
        operation.arguments[0] = JoinPath(prefix(), operation.arguments[0]);
      }
    }
    if (name == "iScan") {
      const size_t target = operation.arguments[0] == "-ir" ? 1 : 0;
      operation.arguments[target] =
          JoinPath(prefix(), operation.arguments[target]);
    }
    if (name == "iScan" && !operations_.empty() &&
        IsRawHistoryOmissionNote(operations_.back())) {
      const bool named = !operations_.back().arguments[1].compare(
          0, sizeof(kNamedRawHistoryOmission) - 1, kNamedRawHistoryOmission);
      const auto text = RawHistoryOmissionText(operation, named);
      for (auto note = operations_.rbegin(); note != operations_.rend();
           ++note) {
        if (note->command != "iNote" || note->arguments.size() != 2 ||
            (note->arguments[0] != "-comment" &&
             note->arguments[0] != "-status") ||
            note->arguments[1] != text ||
            !CanonicalRootScan(operation, named)) {
          break;
        }
        note->raw_history_omitted = true;
        operation.raw_history_omitted = true;
      }
    }
    if (name == "iOverrideScanInterface") {
      for (auto& argument : operation.arguments) {
        if (!argument.empty() && argument[0] == '-') break;
        argument = JoinPath(prefix(), argument);
      }
    }
    for (size_t i = 0; i + 1 < operation.arguments.size(); ++i) {
      if ((name == "iClockOverride" && operation.arguments[i] == "-source") ||
          (name == "iRunLoop" && operation.arguments[i] == "-sck")) {
        operation.arguments[i + 1] =
            JoinPath(prefix(), operation.arguments[i + 1]);
      }
    }
    if ((name == "iTake" || name == "iRelease") && has_callbacks_) {
      const auto& target = operation.arguments.at(0);
      if (callback_scopes_.count(target)) {
        throw std::runtime_error(
            "ownership of logical callback storage is unsupported");
      }
      if (!instance_scopes_.count(target)) {
        for (const auto& bit : ResolvePdlTarget(*netlist_, target).bits) {
          const auto& node = netlist_->nodes.at(bit.node);
          if (node.read_callback || node.write_callback) {
            throw std::runtime_error(
                "ownership of logical callback storage is unsupported");
          }
        }
      }
    }
    std::vector<Operation> callback_ordinary;
    if ((name == "iWrite" || name == "iRead") && has_callbacks_) {
      const auto target =
          ResolvePdlTarget(*netlist_, operation.arguments.at(0));
      const bool read = name == "iRead";
      operation.callback = std::any_of(
          target.bits.begin(), target.bits.end(), [&](const auto& bit) {
            const auto& node = netlist_->nodes.at(bit.node);
            return (read ? node.read_callback : node.write_callback)
                       .has_value() ||
                   (!read && WriteCallbackSource(bit).has_value());
          });
      if (operation.callback) {
        ValidatePdlAccessTarget(*netlist_, target, read);
        if (!history_context_) {
          history_context_ = std::make_unique<z3::context>();
        }
        const auto bits = operation.arguments.size() > 1
                              ? PdlDataValue(*history_context_, *netlist_,
                                             target, operation.arguments[1])
                              : std::string(target.bits.size(), 'x');
        operation.arguments = {operation.arguments[0], "0b" + bits};
        callback_ordinary = CallbackOrdinaryAccesses(operation, target, bits);
        for (const auto& projected : callback_ordinary) {
          CheckCallbackCallerWrite(projected);
        }
      }
    }
    CheckCallbackCallerWrite(operation);
    if (name == "iWrite" || name == "iRead" || name == "iScan") {
      if (!pending_) pending_operation_ = operations_.size();
      pending_ = true;
    }
    if (name == "iState") {
      RememberHistory(operation);
      RememberCallbackState(operation);
    }
    if (name == "iApply") {
      if (callback_failed_) {
        throw std::runtime_error("failed callback queue requires iReset");
      }
      if (ExecuteCallbacks(operation)) {
        operation.callback_execution = true;
        operation.callback_read_execution =
            operation.callback_read_execution || read_callbacks_executed_;
      } else {
        InvalidateReadHistory(operation);
      }
    }
    const bool guarded_state =
        name == "iState" && (operation.arguments.size() != 3 ||
                             operation.arguments[2] == "-LastWrittenValue");
    if (name == "iApply" || name == "iRunLoop" || guarded_state) {
      for (const auto& writes : callback_caller_writes_) {
        for (const auto& [key, value] : writes) {
          const auto [id, index] = key;
          const auto& node = netlist_->nodes.at(id);
          auto [entry, inserted] =
              operation.callback_preserved.try_emplace(id, node.Width(), 'x');
          entry->second.at(node.left >= node.right ? node.left - index
                                                   : index - node.left) = value;
        }
      }
    }
    if (name == "iApply" || name == "iRunLoop" || name == "iReset") {
      callback_held_.clear();
    }
    if (name == "iReset") {
      opaque_history_scan_ = false;
      ++callback_reset_generation_;
      callback_values_.clear();
      callback_failed_ = false;
      for (auto& writes : callback_caller_writes_) writes.clear();
    }
    if (name == "iApply" || name == "iReset") {
      pending_ = false;
      if (callback_stack_.empty()) consumed_callback_operations_.clear();
    }
    if (name == "iRunLoop" && pending_) {
      throw std::runtime_error("iRunLoop has unapplied operations");
    }
    if (name == "iMerge") {
      if (pending_) throw std::runtime_error("iMerge has unapplied operations");
      if (words[1] == "-begin") {
        if (!merges_.empty() && merges_.back().depth == frames_.size()) {
          throw std::runtime_error("nested iMerge must occur through iCall");
        }
        merges_.push_back(
            {frames_.size(), origin, {}, !site || !site->immediate_merge});
      } else if (words[1] == "-end" && !merges_.empty() &&
                 merges_.back().depth == frames_.size()) {
        // IEEE7.9.18(b): direct notes execute before any merged call body.
        ExecuteMergeCalls(merges_.size() - 1);
        merges_.pop_back();
      } else {
        throw std::runtime_error("unmatched iMerge");
      }
    }
    operations_.push_back(std::move(operation));
    for (auto& projected : callback_ordinary) {
      operations_.push_back(std::move(projected));
    }
  }

  std::map<std::pair<size_t, int64_t>, std::optional<BitReference>>
      callback_sources_;
  size_t callback_dependency_budget_ = 4 * 1024 * 1024;
  size_t callback_reset_generation_ = 0;
  bool has_callbacks_ = false, callback_failed_ = false,
       callbacks_executed_ = false;
  std::map<std::pair<size_t, bool>, CallbackBinding> callback_bindings_;
  std::vector<std::pair<size_t, bool>> callback_stack_;
  std::set<size_t> consumed_callback_operations_;
  std::vector<std::map<std::pair<size_t, int64_t>, char>>
      callback_caller_writes_;
  size_t read_callback_depth_ = 0;
  bool read_callbacks_executed_ = false;
  std::set<std::string> callback_scopes_;
  std::map<size_t, size_t> callback_action_ends_;
  bool trace_source_ = false;
  Readback readback_;
  StatusReadback status_readback_;
  HistoryReadback history_readback_;
  std::optional<size_t> modeled_history_root_;
  size_t source_count_ = 0;
  std::vector<SourceAction> actions_;
  std::vector<SourceAction> compounds_;
  std::optional<SourceAction> compound_;
  std::optional<size_t> main_merge_action_;
  const Design& design_;
  const Netlist* netlist_;
  bool scan_clients_loaded_ = false;
  std::optional<bool> named_opaque_root_;
  std::unique_ptr<ScanScheduler> metadata_scheduler_;
  std::set<std::string> tap_clients_;
  std::map<std::string, std::set<std::string>> scan_clients_;
  History read_history_, miscompare_history_, callback_values_, callback_held_;
  using RawHistory = std::map<std::pair<std::string, std::string>, std::string>;
  RawHistory raw_read_history_, raw_miscompare_history_;
  bool opaque_history_scan_ = false;
  std::map<std::pair<std::string, std::string>, size_t> opaque_history_pending_;
  bool zero_status_known_ = true;
  std::unique_ptr<z3::context> history_context_;
  std::map<std::string, const InstanceScope*> instance_scopes_;
  std::string top_;
  std::unique_ptr<Tcl_Interp, decltype(&Tcl_DeleteInterp)> master_;
  std::unique_ptr<Tcl_Interp, decltype(&Tcl_DeleteInterp)> interpreter_;
  std::vector<Site> sites_;
  std::vector<Frame> frames_;
  struct Merge {
    size_t depth;
    std::string origin;
    std::vector<MergeCall> calls;
    bool defer_calls = true;
  };
  std::vector<Merge> merges_;
  std::map<ProcedureKey, Procedure> procedures_;
  std::vector<Operation> operations_;
  std::vector<std::string> warnings_;
  std::string binding_, definition_namespace_, fallback_origin_;
  size_t executed_ = 0, next_procedure_ = 0, pending_operation_ = 0;
  int level_ = 1;
  bool pending_ = false, failed_ = false, finished_ = false;
  bool internal_command_ = false;
  std::map<std::string, std::set<std::string>> module_parameters_;
};

Runtime::Runtime(const Design& design, const std::string& top)
    : impl_(std::make_unique<Impl>(design, top)) {}
Runtime::Runtime(const Design& design, const Netlist& netlist,
                 bool trace_source, Readback readback,
                 StatusReadback status_readback,
                 HistoryReadback history_readback)
    : impl_(std::make_unique<Impl>(design, netlist.top_module, &netlist,
                                   trace_source, std::move(readback),
                                   std::move(status_readback),
                                   std::move(history_readback))) {}
Runtime::~Runtime() = default;
void Runtime::Execute(const std::string& path, const std::string& text) {
  impl_->Execute(path, text);
}
void Runtime::Finish() { impl_->Finish(); }
const std::vector<Operation>& Runtime::operations() const {
  return impl_->operations();
}
std::vector<SourceAction> Runtime::SourceActions(bool whole_commands) const {
  return impl_->SourceActions(whole_commands);
}
const std::vector<std::string>& Runtime::warnings() const {
  return impl_->warnings();
}
int Runtime::Level() const { return impl_->Level(); }
bool Runtime::CallbacksExecuted() const { return impl_->CallbacksExecuted(); }
bool Runtime::ReadCallbacksExecuted() const {
  return impl_->ReadCallbacksExecuted();
}
}  // namespace pattern
