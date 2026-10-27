// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_merge.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "pattern/pattern_values.h"

namespace pattern {
namespace {
class Merger {
 public:
  Merger(const Netlist& netlist, const std::vector<Operation>& operations,
         bool combine)
      : netlist_(netlist), operations_(operations), combine_(combine) {}

  std::vector<Operation> Run() {
    std::vector<Operation> result;
    while (position_ < operations_.size()) AppendNext(result);
    return result;
  }

 private:
  void AppendNext(std::vector<Operation>& result) {
    if (operations_[position_].command != "iMerge") {
      result.push_back(operations_[position_++]);
      return;
    }
    auto merged = Block();
    result.insert(result.end(), std::make_move_iterator(merged.begin()),
                  std::make_move_iterator(merged.end()));
  }

  bool Batchable(const std::vector<Operation>& stream,
                 std::set<size_t>& targets) {
    if (!combine_ || stream.size() < 2 || stream.back().command != "iApply" ||
        !stream.back().arguments.empty()) {
      return false;
    }
    // Partial/alias writes and stateful calls keep their original
    // fill/barriers.
    try {
      for (size_t i = 0; i + 1 < stream.size(); ++i) {
        const auto& op = stream[i];
        if (op.command != "iWrite" || op.arguments.size() != 2) return false;
        const auto target = ResolvePdlTarget(netlist_, op.arguments[0]);
        const auto& node = netlist_.nodes.at(target.declaration);
        if (node.kind != "ScanRegister" || target.bits.size() != node.Width() ||
            PdlDataValue(context_, netlist_, target, op.arguments[1])
                    .find('x') != std::string::npos) {
          return false;
        }
        targets.insert(target.declaration);
      }
    } catch (const std::runtime_error&) {
      // Let normal scheduling report invalid operands with their source
      // context.
      return false;
    }
    return true;
  }

  std::vector<Operation> Block() {
    const auto& begin = operations_.at(position_++);
    if (begin.arguments != std::vector<std::string>{"-begin"}) {
      throw std::runtime_error(begin.origin + ": unmatched iMerge");
    }
    const size_t depth = begin.call_stack.size();
    std::vector<Operation> result;
    std::vector<std::vector<Operation>> streams;
    std::string invocation;
    while (position_ < operations_.size()) {
      const auto& operation = operations_[position_];
      if (operation.command == "iMerge" &&
          operation.call_stack.size() == depth &&
          operation.arguments == std::vector<std::string>{"-end"}) {
        break;
      }
      if (operation.call_stack.size() == depth &&
          operation.command == "iNote") {
        result.push_back(operation);
        ++position_;
        continue;
      }
      if (operation.call_stack.size() <= depth) {
        throw std::runtime_error(operation.origin + ": invalid iMerge body");
      }
      const auto& call = operation.call_stack[depth];
      if (streams.empty() || call != invocation) {
        streams.emplace_back();
        invocation = call;
      }
      AppendNext(streams.back());
    }
    if (position_ == operations_.size()) {
      throw std::runtime_error(begin.origin + ": unclosed iMerge");
    }
    ++position_;

    std::set<size_t> occupied;
    std::vector<Operation> batch;
    Operation apply;
    const auto flush = [&] {
      if (batch.empty()) return;
      result.insert(result.end(), std::make_move_iterator(batch.begin()),
                    std::make_move_iterator(batch.end()));
      result.push_back(std::move(apply));
      batch.clear();
      occupied.clear();
    };
    for (auto& stream : streams) {
      std::set<size_t> targets;
      if (!Batchable(stream, targets)) {
        flush();
        result.insert(result.end(), std::make_move_iterator(stream.begin()),
                      std::make_move_iterator(stream.end()));
        continue;
      }
      if (std::any_of(targets.begin(), targets.end(),
                      [&](size_t target) { return occupied.count(target); })) {
        flush();
      }
      occupied.insert(targets.begin(), targets.end());
      apply = std::move(stream.back());
      stream.pop_back();
      batch.insert(batch.end(), std::make_move_iterator(stream.begin()),
                   std::make_move_iterator(stream.end()));
    }
    flush();
    return result;
  }

  const Netlist& netlist_;
  const std::vector<Operation>& operations_;
  bool combine_;
  size_t position_ = 0;
  z3::context context_;
};
}  // namespace

std::vector<Operation> MergeOperations(const Netlist& netlist,
                                       const std::vector<Operation>& operations,
                                       bool combine) {
  return Merger(netlist, operations, combine).Run();
}
}  // namespace pattern
