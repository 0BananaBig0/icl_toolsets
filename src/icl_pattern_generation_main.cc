// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <tcl.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "file_output.h"
#include "pattern_flatten.h"
#include "pattern_runtime.h"
#include "pattern_scheduler.h"
#include "pattern_tap.h"
#include "pattern_values.h"
#include "pdl_document.h"
#include "text_input.h"

namespace {
struct Options {
  std::vector<std::string> icl, pdl;
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
std::string Generate(const pattern::Netlist& netlist,
                     const std::vector<pattern::Operation>& operations) {
  pattern::ScanScheduler scheduler(netlist);
  z3::context context;
  std::vector<pattern::Operation> pending;
  std::string result =
      "iPDLLevel 0\n" + Command({"iProcsForModule", netlist.top_module});
  for (const auto& operation : operations) {
    try {
      if (operation.command == "iWrite" || operation.command == "iRead") {
        pending.push_back(operation);
      } else if (operation.command == "iApply") {
        if (!operation.arguments.empty()) {
          throw std::runtime_error(
              "iApply options require simultaneous transaction scheduling");
        }
        const auto steps = scheduler.Apply(pending);
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
          result += "iApply\n";
        }
        if (steps.empty()) result += "iApply\n";
        pending.clear();
      } else if (operation.command == "iReset") {
        if (!pending.empty()) {
          throw std::runtime_error(
              "iReset with pending actions requires queued-state integration");
        }
        scheduler.Reset();
        result += "iReset\n";
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
        result += Command({"iState", name, "0b" + bits});
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
  const pdl::Document validated(result);
  if (!validated.ok()) {
    throw std::runtime_error("generated PDL failed shared parser: " +
                             validated.error_message());
  }
  return result;
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
           "  --top_module NAME    Select the elaborated top (default: unique "
           "root)\n"
           "  --output PATH        Output PDL (default: out/<top>.pdl)\n"
           "  --debug              Also write debug/<top>.icl beside output\n"
           "  --log PATH           Copy diagnostics to a file\n"
           "  --local_reset PATH   Reserved; reset-only rewriting is not yet "
           "implemented\n"
           "Current scheduling: ordinary single SI/SO, scan registers, muxes,\n"
           "logic-controlled paths and local resets; literal "
           "iWrite/iApply/iState.\n"
           "TAP, explicit interface controls, merges and clocks are "
           "rejected.\n";
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
    CheckPaths(inputs, {options.output, options.log, options.local_reset});
    // Log output becomes writable only after alias validation.
    log_path = options.log;
    pattern::Design design;
    for (const auto& path : options.icl) design.AddIcl(path, Read(path));
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
    if (!options.local_reset.empty()) {
      throw std::runtime_error(
          "--local_reset rewrite integration is not implemented");
    }
    const auto network = pattern::ElaborateTapNetwork(design, top);
    pattern::Runtime runtime(design, network.netlist);
    for (const auto& path : options.pdl) runtime.Execute(path, Read(path));
    runtime.Finish();
    const auto pdl = Generate(network.netlist, runtime.operations());
    for (const auto& warning : design.warnings()) messages += warning + "\n";
    for (const auto& warning : runtime.warnings()) messages += warning + "\n";
    // All parsing, execution, scheduling and replay finish before publication.
    const auto success_messages = messages + "info: generated " + output + "\n";
    std::vector<std::pair<std::string, std::string>> artifacts;
    if (!debug.empty()) {
      artifacts.emplace_back(debug, pattern::SerializeIcl(network.netlist));
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
