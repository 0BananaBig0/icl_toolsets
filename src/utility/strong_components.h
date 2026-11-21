// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_UTILITY_STRONG_COMPONENTS_H_
#define ICL_TOOLSETS_SRC_UTILITY_STRONG_COMPONENTS_H_

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace toolsets {
// All vertices and parents belong to the supplied graph. Return sources first.
inline std::vector<std::vector<size_t>> StrongComponents(
    const std::vector<size_t>& vertices,
    const std::map<size_t, std::set<size_t>>& parents,
    const std::map<size_t, std::vector<size_t>>& children) {
  // Iterative Kosaraju keeps graph depth off the C++ call stack.
  std::set<size_t> visited;
  std::vector<size_t> finished;
  for (size_t root : vertices) {
    std::vector<std::pair<size_t, bool>> pending{{root, false}};
    while (!pending.empty()) {
      const auto [id, expanded] = pending.back();
      pending.pop_back();
      if (expanded) {
        finished.push_back(id);
      } else if (visited.insert(id).second) {
        pending.emplace_back(id, true);
        const auto found = children.find(id);
        if (found != children.end()) {
          for (size_t child : found->second) pending.emplace_back(child, false);
        }
      }
    }
  }
  std::vector<std::vector<size_t>> result;
  visited.clear();
  for (auto root = finished.rbegin(); root != finished.rend(); ++root) {
    if (!visited.insert(*root).second) continue;
    std::vector<size_t> component, pending{*root};
    while (!pending.empty()) {
      const size_t id = pending.back();
      pending.pop_back();
      component.push_back(id);
      for (size_t parent : parents.at(id)) {
        if (visited.insert(parent).second) pending.push_back(parent);
      }
    }
    std::sort(component.begin(), component.end());
    result.push_back(std::move(component));
  }
  return result;
}
}  // namespace toolsets
#endif  // ICL_TOOLSETS_SRC_UTILITY_STRONG_COMPONENTS_H_
