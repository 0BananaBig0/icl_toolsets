// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_readback.h"

#include <algorithm>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_resources.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

namespace pattern {
namespace {
class ModelReadback {
 public:
  ModelReadback(const Netlist& netlist, bool physical)
      : netlist_(netlist),
        physical_(physical),
        scheduler_(netlist),
        reservations_(netlist) {
    if (std::any_of(
            netlist.nodes.begin(), netlist.nodes.end(), [](const auto& node) {
              return node.model == ModelKind::kRegister &&
                     node.kind != "DataRegister" && node.kind != "ScanRegister";
            })) {
      throw std::runtime_error("modeled readback excludes unsupported storage");
    }
  }

  const ReadbackUpdates* Read(const std::vector<Operation>& operations,
                              bool clear_status) {
    if (error_) std::rethrow_exception(error_);
    try {
      // Reject unsupported histories before exposing any observation delta.
      for (size_t i = cursor_; i < operations.size(); ++i) {
        const auto& op = operations[i];
        if (op.command != "iWrite" && op.command != "iRead" &&
            op.command != "iApply" && op.command != "iState" &&
            op.command != "iReset" && op.command != "iTake" &&
            op.command != "iRelease" && op.command != "iNote") {
          throw std::runtime_error(op.origin +
                                   ": modeled readback does not support " +
                                   op.command + " histories");
        }
      }
      updates_.bits.clear();
      positions_.clear();
      for (; cursor_ < operations.size(); ++cursor_) {
        const auto& op = operations[cursor_];
        try {
          Process(op);
        } catch (const std::exception& error) {
          throw std::runtime_error(op.origin + ": " + error.what());
        }
      }
      updates_.failures = failures_;
      if (clear_status && failures_) failures_ = 0;
      return &updates_;
    } catch (...) {
      // A failed model execution cannot be resumed from partially applied
      // state.
      error_ = std::current_exception();
      throw;
    }
  }

 private:
  void Observe(const std::vector<BitReference>& bits,
               const std::string& expected, const std::string& actual) {
    for (size_t i = 0; i < bits.size(); ++i) {
      const char mismatch = expected[i] == 'x'         ? '0'
                            : actual[i] == 'x'         ? 'x'
                            : actual[i] == expected[i] ? '0'
                                                       : '1';
      if (mismatch == 'x') failures_.reset();
      if (mismatch == '1' && failures_) ++*failures_;
      const auto& bit = bits[i];
      const auto [position, inserted] = positions_.emplace(
          std::make_pair(bit.node, bit.index), updates_.bits.size());
      const ReadbackUpdate update{bit.node, bit.index, actual[i], mismatch};
      if (inserted) {
        updates_.bits.push_back(update);
      } else {
        updates_.bits[position->second] = update;
      }
    }
  }

  void Process(const Operation& op) {
    reservations_.Enter(op.call_stack);
    const auto& args = op.arguments;
    if (op.command == "iWrite" || op.command == "iRead") {
      const auto target = ResolvePdlTarget(netlist_, args.at(0));
      ValidatePdlAccessTarget(netlist_, target, op.command == "iRead");
      const auto bits = args.size() == 2
                            ? PdlDataValue(context_, netlist_, target, args[1])
                            : std::string(target.bits.size(), 'x');
      pending_.Add(op, target, bits);
    } else if (op.command == "iApply") {
      const auto& accesses = pending_.Operations();
      const auto [bits, expected] = pending_.ReadModel();
      std::map<std::pair<size_t, int64_t>, size_t> requested;
      for (size_t i = 0; i < bits.size(); ++i) {
        requested.emplace(std::make_pair(bits[i].node, bits[i].index), i);
      }
      std::string actual(bits.size(), '?');
      const ScanScheduler::PioObservation observe =
          [&](const auto& observed_bits, const auto& values) {
            for (size_t i = 0; i < observed_bits.size(); ++i) {
              const auto& bit = observed_bits[i];
              const auto found = requested.find({bit.node, bit.index});
              if (found != requested.end() && actual[found->second] == '?') {
                actual[found->second] = values[i];
              }
            }
          };
      scheduler_.Apply(
          accesses, ValidatePdlApplyOptions(args), reservations_.Values(), true,
          physical_, bits.empty() ? ScanScheduler::PioObservation{} : observe);
      if (actual.find('?') != std::string::npos) {
        throw std::runtime_error(
            "modeled apply did not observe every read bit");
      }
      reservations_.Assigned(accesses, scheduler_);
      Observe(bits, expected, actual);
      pending_.Clear();
    } else if (op.command == "iReset") {
      scheduler_.Reset(!args.empty(), physical_);
      reservations_.Reset();
      pending_.Clear();
    } else if (op.command == "iState") {
      const auto target = ResolvePdlTarget(netlist_, args.at(0));
      const auto bits = PdlNumber(context_, args.at(1), target.bits.size());
      if (args.size() != 3 || args[2] == "-LastWrittenValue") {
        scheduler_.Assume(args[0], bits);
        pending_.Assume(target, bits);
        reservations_.Assigned({op}, scheduler_);
      } else {
        for (const auto& bit : target.bits) {
          ValidateStateTarget(netlist_, bit.node);
          const auto found = positions_.find({bit.node, bit.index});
          if (found == positions_.end()) continue;
          auto& update = updates_.bits[found->second];
          if (args[2] == "-LastReadValue") {
            update.read = '?';
          } else {
            update.miscompare = '?';
          }
        }
      }
    } else if (op.command == "iTake" || op.command == "iRelease") {
      reservations_.Change(op);
    }
  }

  const Netlist& netlist_;
  bool physical_;
  z3::context context_;
  ScanScheduler scheduler_;
  ResourceReservations reservations_;
  QueuedAccesses pending_;
  size_t cursor_ = 0;
  std::optional<uint64_t> failures_{0};
  ReadbackUpdates updates_;
  std::map<std::pair<size_t, int64_t>, size_t> positions_;
  std::exception_ptr error_;
};
}  // namespace

Runtime::Readback MakeModeledReadback(const Netlist& netlist) {
  return MakeModeledReadback(netlist, false);
}
Runtime::Readback MakeModeledReadback(const Netlist& netlist, bool physical) {
  return [&netlist, physical, model = std::shared_ptr<ModelReadback>{}](
             const std::vector<Operation>& operations,
             bool clear_status) mutable -> const ReadbackUpdates* {
    if (!model) model = std::make_shared<ModelReadback>(netlist, physical);
    return model->Read(operations, clear_status);
  };
}
}  // namespace pattern
