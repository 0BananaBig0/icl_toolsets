// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "pattern/pattern_flatten.h"
#include "utility/json.h"

namespace {
icl::Json InspectModule(pattern::Design& design, const std::string& name) {
  std::string stage = "elaborate";
  icl::Json::Members result{{"name", icl::Json::String(name)}};
  try {
    const auto original = pattern::Elaborate(design, "::" + name);
    stage = "serialize";
    const auto flat = pattern::SerializeIcl(original);
    stage = "flat_load";
    pattern::Design restored;
    restored.AddIcl(name + ".flat.icl", flat);
    stage = "flat_elaborate";
    const auto roundtrip = pattern::Elaborate(restored, "");
    stage = "canonical";
    if (pattern::SerializeIcl(roundtrip) != flat) {
      throw std::runtime_error("canonical flat bytes differ after roundtrip");
    }
    result.emplace_back("ok", icl::Json::Bool(true));
    result.emplace_back("nodes", icl::Json::Int(original.nodes.size()));
    result.emplace_back("flat_bytes", icl::Json::Int(flat.size()));
  } catch (const std::runtime_error& error) {
    result.emplace_back("ok", icl::Json::Bool(false));
    result.emplace_back("error", icl::Json::String(error.what()));
  }
  result.emplace_back("stage", icl::Json::String(stage));
  return icl::Json::Object(std::move(result));
}

icl::Json InspectFile(const std::string& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) throw std::runtime_error("cannot open " + path);
  const std::string source{std::istreambuf_iterator<char>(input), {}};
  if (input.bad()) throw std::runtime_error("cannot read " + path);
  pattern::Design design;
  icl::Json::Members result{{"path", icl::Json::String(path)}};
  icl::Json::Elements modules;
  try {
    design.AddIcl(path, source);
  } catch (const std::runtime_error& error) {
    result.emplace_back("load_error", icl::Json::String(error.what()));
    result.emplace_back("modules", icl::Json::Array({}));
    return icl::Json::Object(std::move(result));
  }
  for (const auto& module : design.modules()) {
    modules.push_back(InspectModule(design, module.first));
  }
  result.emplace_back("modules", icl::Json::Array(std::move(modules)));
  return icl::Json::Object(std::move(result));
}
}  // namespace

int main(int argc, char** argv) {
  try {
    for (int i = 1; i < argc; ++i) {
      std::cout << InspectFile(argv[i]).Serialize() << '\n';
    }
    std::cout.flush();
    if (!std::cout) throw std::runtime_error("cannot write inventory");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
