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
      bool pending = false;
      for (const auto& op : stream) {
        if (op.command == "iApply") {
          if (!pending || !op.arguments.empty()) return false;
          pending = false;
          continue;
        }
        const bool read = op.command == "iRead";
        if ((!read && op.command != "iWrite") ||
            (op.arguments.size() != 2 && !(read && op.arguments.size() == 1))) {
          return false;
        }
        const auto target = ResolvePdlTarget(netlist_, op.arguments[0]);
        const auto& node = netlist_.nodes.at(target.declaration);
        if (node.kind != "ScanRegister" || target.bits.size() != node.Width()) {
          return false;
        }
        const auto bits =
            op.arguments.size() == 1
                ? std::string(node.Width(), 'x')
                : PdlDataValue(context_, netlist_, target, op.arguments[1]);
        if (!read && bits.find('x') != std::string::npos) {
          return false;
        }
        targets.insert(target.declaration);
        pending = true;
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
    std::vector<std::vector<Operation>> batch;
    const auto flush = [&] {
      if (batch.empty()) return;
      std::vector<std::pair<size_t, size_t>> active;
      for (size_t i = 0; i < batch.size(); ++i) active.emplace_back(i, 0);
      // IEEE 7.9.18(i): preserve each call's apply order. Retire exhausted
      // streams so unequal lengths do not cause repeated empty scans.
      while (!active.empty()) {
        std::vector<std::pair<size_t, size_t>> next;
        Operation apply;
        for (auto [id, position] : active) {
          auto& stream = batch[id];
          while (stream[position].command != "iApply") {
            result.push_back(std::move(stream[position++]));
          }
          apply = std::move(stream[position++]);
          if (position != stream.size()) next.emplace_back(id, position);
        }
        result.push_back(std::move(apply));
        active = std::move(next);
      }
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
      batch.push_back(std::move(stream));
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
