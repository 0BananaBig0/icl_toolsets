// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

#include "pattern_flatten.h"

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
        netlist.NodeAt("i4999.r").Width() != 8) {
      throw std::runtime_error("large hierarchy elaboration mismatch");
    }
    const auto text = pattern::SerializeIcl(netlist);
    const auto serialized = std::chrono::steady_clock::now();
    pattern::Design flat_design;
    flat_design.AddIcl("flat.icl", text);
    const auto flat_netlist = pattern::Elaborate(flat_design, "Top");
    const auto replayed = std::chrono::steady_clock::now();
    if (pattern::SerializeIcl(flat_netlist) != text) {
      throw std::runtime_error("large hierarchy replay mismatch");
    }
    std::cout
        << "instances=" << kInstances << " source_bytes=" << source.size()
        << " nodes=" << netlist.nodes.size() << " parse_ms="
        << std::chrono::duration<double, std::milli>(parsed - start).count()
        << " elaborate_ms="
        << std::chrono::duration<double, std::milli>(elaborated - parsed)
               .count()
        << " flat_bytes=" << text.size() << " serialize_ms="
        << std::chrono::duration<double, std::milli>(serialized - elaborated)
               .count()
        << " reimport_ms="
        << std::chrono::duration<double, std::milli>(replayed - serialized)
               .count()
        << '\n';
    constexpr size_t kWidth = 1048576;
    constexpr size_t kContributors = 10000;
    std::string sparse_source =
        "Module Sparse { DataInPort en; DataOutPort out { Source 1'b0; "
        "Enable en; } OneHotDataGroup g[1048575:0] { ";
    for (size_t i = 0; i < kContributors; ++i) sparse_source += "Port out; ";
    sparse_source += "} }";
    pattern::Design sparse_design;
    sparse_design.AddIcl("sparse_group.icl", sparse_source);
    const auto sparse_start = std::chrono::steady_clock::now();
    const auto sparse = pattern::Elaborate(sparse_design, "Sparse");
    const auto& group = sparse.NodeAt("g");
    if (group.Width() != kWidth ||
        group.one_hot_sources.size() != kContributors ||
        group.OneHotSourceBit(0, 0).constant != 'x' ||
        group.OneHotSourceBit(0, kWidth - 1).node !=
            sparse.node_ids.at("out")) {
      throw std::runtime_error("sparse one-hot group mapping mismatch");
    }
    std::cout << "sparse_width=" << kWidth << " contributors=" << kContributors
              << " source_bytes=" << sparse_source.size() << " elaborate_ms="
              << std::chrono::duration<double, std::milli>(
                     std::chrono::steady_clock::now() - sparse_start)
                     .count()
              << '\n';
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
