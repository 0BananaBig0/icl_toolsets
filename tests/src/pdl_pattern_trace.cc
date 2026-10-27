// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
// Static operation comparison only; this never executes input Tcl.
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

#include "pdl/pdl_document.h"
#include "utility/json.h"
#include "utility/text_input.h"

namespace {
using icl::Json;

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
    if (argc != 3) {
      throw std::runtime_error("usage: pdl_pattern_trace FILE ENTRY");
    }
    std::ifstream input(argv[1], std::ios::binary);
    std::string source;
    if (!input || !toolsets::ReadTextStream(input, &source)) {
      throw std::runtime_error("cannot read input");
    }
    std::cout << Trace(pdl::Document(source), argv[2]).Serialize() << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
