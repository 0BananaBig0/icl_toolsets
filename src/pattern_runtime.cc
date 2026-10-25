// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_runtime.h"

#include <tcl.h>

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "pattern_netlist.h"
#include "pdl_document.h"

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
       const Netlist* netlist = nullptr)
      : design_(design),
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
    Tcl_SetRecursionLimit(interpreter_.get(), 128);
    Tcl_CreateObjTrace(interpreter_.get(), 0, 0, Trace, this, nullptr);
    for (const auto& name : pdl::Keywords()) {
      if (!pdl::FindCommandSpec(name)) continue;
      Tcl_CreateObjCommand(interpreter_.get(), name.c_str(), Dispatch, this,
                           nullptr);
    }
    Tcl_CreateObjCommand(interpreter_.get(), "::icl_pattern_dispatch", Dispatch,
                         this, nullptr);
    frames_.push_back({});
    if (netlist) {
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
    const auto script = Instrument(path, document);
    Tcl_Time deadline;
    Tcl_GetTime(&deadline);
    deadline.sec += 5;
    Tcl_LimitSetTime(interpreter_.get(), &deadline);
    Tcl_LimitTypeSet(interpreter_.get(), TCL_LIMIT_TIME);
    const int result =
        Tcl_EvalEx(interpreter_.get(), script.data(),
                   static_cast<int>(script.size()), TCL_EVAL_GLOBAL);
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
  const std::vector<std::string>& warnings() const { return warnings_; }

 private:
  struct Site {
    std::string path;
    std::shared_ptr<const pdl::Document> document;
    pdl::ByteRange range;
    const pdl::Command* command;
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
  struct Procedure {
    std::string tcl_name, origin;
    int level;
    std::vector<std::string> parameters;
  };
  using ProcedureKey = std::tuple<std::string, std::string, std::string>;

  std::string Instrument(const std::string& path,
                         const std::shared_ptr<const pdl::Document>& document) {
    struct Edit {
      size_t end;
      std::string text;
    };
    std::map<size_t, Edit> edits;
    std::function<void(const std::vector<pdl::Command>&)> visit;
    visit = [&](const auto& commands) {
      for (const auto& command : commands) {
        if (command.words.empty()) continue;
        const auto& first = command.words.front();
        const auto name = CommandName(first.value);
        if (first.literal && !first.expanded && pdl::FindCommandSpec(name)) {
          const size_t id = sites_.size();
          sites_.push_back({path, document, command.range, &command});
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
        }
        for (const auto& word : command.words) {
          visit(word.script);
          visit(word.substitutions);
        }
      }
    };
    visit(document->commands());
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
      self.Command(arguments, origin, site ? &*site : nullptr);
      return TCL_OK;
    } catch (const std::exception& error) {
      const std::string message = origin + ": " + error.what();
      Tcl_SetObjResult(
          interpreter,
          Tcl_NewStringObj(message.data(), static_cast<int>(message.size())));
      return TCL_ERROR;
    }
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
      if (name == "icl_pattern_dispatch" || pdl::FindCommandSpec(name)) {
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

  const ModuleDefinition& InstanceModule(const std::string& path) const {
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
      if (!design_.modules().count(binding_)) {
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
      for (const auto* node : design_.modules().at(binding_).ast->body()) {
        const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
        if (statement &&
            (statement->named_kind() == icl::NamedKind::kParameter ||
             statement->named_kind() == icl::NamedKind::kLocalParameter)) {
          parameters.push_back(statement->name());
        }
      }
      bool valid = false;
      const auto formals = pdl::SplitList(words[2], &valid);
      if (!valid) throw std::runtime_error("invalid iProc argument list");
      for (const auto& formal : formals) {
        const auto fields = pdl::SplitList(formal, &valid);
        if (!valid || fields.empty()) {
          throw std::runtime_error("invalid iProc argument");
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
          key, Procedure{tcl_name, origin, level_, std::move(parameters)});
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
      call.insert(call.end(), words.begin() + 2, words.end());
      frames_.push_back({instance, "", "", invocation, 0, definition.level});
      const size_t merge_count = merges_.size();
      try {
        EvalWords(call);
        if (merges_.size() != merge_count) {
          throw std::runtime_error("iProc returned with unclosed iMerge");
        }
        frames_.pop_back();
      } catch (const std::exception& error) {
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
      return;
    }
    if (name == "iGetReadData" || name == "iGetMiscompares" ||
        name == "iGetStatus") {
      throw std::runtime_error(
          name +
          " requires hardware readback; unavailable during static generation");
    }
    Operation operation{name, {words.begin() + 1, words.end()}, origin, {}};
    for (const auto& frame : frames_) {
      if (!frame.invocation.empty()) {
        operation.call_stack.push_back(frame.invocation);
      }
    }
    if (name == "iWrite" || name == "iRead" || name == "iState" ||
        name == "iTake" || name == "iRelease" || name == "iClock" ||
        name == "iClockOverride") {
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
    if (name == "iWrite" || name == "iRead" || name == "iScan") {
      if (!pending_) pending_operation_ = operations_.size();
      pending_ = true;
    }
    if (name == "iApply") pending_ = false;
    if (name == "iRunLoop" && pending_) {
      throw std::runtime_error("iRunLoop has unapplied operations");
    }
    if (name == "iMerge") {
      if (pending_) throw std::runtime_error("iMerge has unapplied operations");
      if (words[1] == "-begin") {
        merges_.push_back({frames_.size(), origin});
      } else if (words[1] == "-end" && !merges_.empty() &&
                 merges_.back().depth == frames_.size()) {
        merges_.pop_back();
      } else {
        throw std::runtime_error("unmatched iMerge");
      }
    }
    operations_.push_back(std::move(operation));
  }

  const Design& design_;
  std::map<std::string, const InstanceScope*> instance_scopes_;
  std::string top_;
  std::unique_ptr<Tcl_Interp, decltype(&Tcl_DeleteInterp)> master_;
  std::unique_ptr<Tcl_Interp, decltype(&Tcl_DeleteInterp)> interpreter_;
  std::vector<Site> sites_;
  std::vector<Frame> frames_;
  struct Merge {
    size_t depth;
    std::string origin;
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
};

Runtime::Runtime(const Design& design, const std::string& top)
    : impl_(std::make_unique<Impl>(design, top)) {}
Runtime::Runtime(const Design& design, const Netlist& netlist)
    : impl_(std::make_unique<Impl>(design, netlist.top_module, &netlist)) {}
Runtime::~Runtime() = default;
void Runtime::Execute(const std::string& path, const std::string& text) {
  impl_->Execute(path, text);
}
void Runtime::Finish() { impl_->Finish(); }
const std::vector<Operation>& Runtime::operations() const {
  return impl_->operations();
}
const std::vector<std::string>& Runtime::warnings() const {
  return impl_->warnings();
}
}  // namespace pattern
