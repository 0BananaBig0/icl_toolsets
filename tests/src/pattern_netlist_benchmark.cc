// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

#include "pattern_netlist.h"

int main() {
  try {
    constexpr size_t kInstances = 5000;
    std::string source = R"(
Module Instrument {
  Parameter W = 32;
  ScanInPort si;
  ScanRegister r[$W-1:0] { ScanInSource si; ResetValue 0; }
  ScanOutPort so { Source r[0]; }
}
Module Top {
  ScanInPort si;
)";
    for (size_t i = 0; i < kInstances; ++i) {
      source += "Instance i" + std::to_string(i) +
                " Of Instrument { Parameter W = " + std::to_string(i % 64 + 1) +
                "; InputPort si = " +
                (i ? "i" + std::to_string(i - 1) + ".so" : "si") + "; }\n";
    }
    source += "ScanOutPort so { Source i4999.so; } }\n";
    const auto start = std::chrono::steady_clock::now();
    pattern::Design design;
    design.AddIcl("benchmark.icl", source);
    const auto parsed = std::chrono::steady_clock::now();
    const auto netlist = pattern::Elaborate(design, "Top");
    const auto elaborated = std::chrono::steady_clock::now();
    if (netlist.scopes.size() != kInstances + 1 ||
        netlist.nodes.size() != 3 * kInstances + 2 ||
        netlist.NodeAt("i4999.r").Width() != 8)
      throw std::runtime_error("large hierarchy elaboration mismatch");
    std::cout
        << "instances=" << kInstances << " source_bytes=" << source.size()
        << " nodes=" << netlist.nodes.size() << " parse_ms="
        << std::chrono::duration<double, std::milli>(parsed - start).count()
        << " elaborate_ms="
        << std::chrono::duration<double, std::milli>(elaborated - parsed)
               .count()
        << '\n';
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
