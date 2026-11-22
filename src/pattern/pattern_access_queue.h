// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_QUEUE_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_QUEUE_H_

#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "pattern/pattern_netlist.h"
#include "pattern/pattern_runtime.h"
#include "pattern/pattern_values.h"

namespace pattern {
class QueuedAccesses {
 public:
  bool Empty() const { return operations_.empty(); }
  void Clear() {
    operations_.clear();
    values_.clear();
    references_.clear();
  }
  void Add(const pattern::Operation& operation,
           const pattern::PdlTarget& target, std::string bits) {
    const size_t index = operations_.size();
    operations_.push_back(operation);
    values_.push_back(std::move(bits));
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      references_[{bit.node, bit.index, operation.command == "iRead"}]
          .push_back({index, i, bit.inverted});
    }
  }
  void Assume(const pattern::PdlTarget& target, const std::string& bits) {
    for (size_t i = 0; i < target.bits.size(); ++i) {
      const auto& bit = target.bits[i];
      const char value = Invert(bits[i], bit.inverted);
      for (bool read : {false, true}) {
        const auto found = references_.find({bit.node, bit.index, read});
        if (found == references_.end()) continue;
        auto& references = found->second;
        const auto last = references.back();
        // Retire overwritten references once, avoiding repeated queue scans.
        for (const auto& ref : references) {
          values_[ref.operation][ref.offset] = 'x';
        }
        values_[last.operation][last.offset] = Invert(value, last.inverted);
        references.assign(1, last);
      }
    }
  }
  const std::vector<pattern::Operation>& Operations() {
    for (size_t i = 0; i < operations_.size(); ++i) {
      operations_[i].arguments.resize(2);
      operations_[i].arguments[1] = "0b" + values_[i];
    }
    return operations_;
  }

  std::pair<std::vector<BitReference>, std::string> ReadModel() const {
    std::pair<std::vector<BitReference>, std::string> result;
    for (const auto& [key, references] : references_) {
      const auto [node, index, read] = key;
      if (!read) continue;
      const auto& last = references.back();
      result.first.push_back({node, index, false});
      result.second +=
          Invert(values_[last.operation][last.offset], last.inverted);
    }
    return result;
  }

 private:
  struct Reference {
    size_t operation, offset;
    bool inverted;
  };
  static char Invert(char bit, bool inverted) {
    return inverted && bit != 'x' ? (bit == '1' ? '0' : '1') : bit;
  }
  std::vector<pattern::Operation> operations_;
  std::vector<std::string> values_;
  std::map<std::tuple<size_t, int64_t, bool>, std::vector<Reference>>
      references_;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_ACCESS_QUEUE_H_
