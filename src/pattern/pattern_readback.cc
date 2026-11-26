// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_readback.h"

#include <algorithm>
#include <exception>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "pattern/pattern_access_queue.h"
#include "pattern/pattern_opaque.h"
#include "pattern/pattern_resources.h"
#include "pattern/pattern_scheduler.h"
#include "pattern/pattern_state.h"
#include "pattern/pattern_values.h"

namespace pattern {
bool CanUsePrimaryCallbackReadback(const Netlist& netlist) {
  bool callback = false, writes = false;
  if (!netlist.taps.empty()) return false;
  for (const auto& node : netlist.nodes) {
    if (node.scan_interface || node.model == ModelKind::kAccessLink ||
        node.kind.rfind("Scan", 0) == 0 ||
        node.kind.find("Clock") != std::string::npos ||
        node.kind.find("TCK") != std::string::npos) {
      return false;
    }
    callback = callback || node.write_callback.has_value();
    writes = writes || (node.kind == "DataRegister" &&
                        node.inputs.count("WriteDataSource"));
  }
  return callback && writes;
}
namespace {
std::optional<Netlist> ObservationNetlist(const Netlist& netlist) {
  if (std::none_of(netlist.nodes.begin(), netlist.nodes.end(),
                   [](const auto& node) {
                     return node.read_callback || node.write_callback;
                   })) {
    return std::nullopt;
  }
  auto result = netlist;
  for (auto& node : result.nodes) {
    if (!node.read_callback && !node.write_callback) continue;
    const auto reset = node.constants.find("ResetValue");
    if (reset != node.constants.end()) {
      reset->second.data.assign(node.Width(), 'x');
    }
  }
  return result;
}
class ModelReadback {
 public:
  ModelReadback(const Netlist& netlist, bool physical)
      : observation_netlist_(ObservationNetlist(netlist)),
        netlist_(observation_netlist_ ? *observation_netlist_ : netlist),
        physical_(physical),
        reservations_(netlist_) {
    const bool opaque =
        netlist.scopes.size() == 1 && netlist.taps.empty() &&
        std::any_of(
            netlist.nodes.begin(), netlist.nodes.end(),
            [](const auto& node) { return node.scan_interface.has_value(); }) &&
        std::none_of(
            netlist.nodes.begin(), netlist.nodes.end(), [](const auto& node) {
              return node.model == ModelKind::kRegister ||
                     node.kind == "DataInPort" || node.kind == "DataOutPort";
            });
    if (opaque) {
      opaque_ = std::make_unique<OpaqueScanQueue>(DescribeOpaqueScans(netlist));
    } else {
      scheduler_ = std::make_unique<ScanScheduler>(netlist_);
    }
    if (!physical && CanUsePrimaryCallbackReadback(netlist)) {
      planning_ = std::make_unique<ScanScheduler>(netlist);
    }
    if (std::any_of(
            netlist.nodes.begin(), netlist.nodes.end(), [](const auto& node) {
              return node.model == ModelKind::kRegister &&
                     node.kind != "DataRegister" && node.kind != "ScanRegister";
            })) {
      throw std::runtime_error("modeled readback excludes unsupported storage");
    }
    callback_storage_.resize(netlist.nodes.size(), false);
    modeled_children_ =
        !opaque_ &&
        std::any_of(
            netlist.nodes.begin(), netlist.nodes.end(), [](const auto& node) {
              return node.scope && node.scan_interface &&
                     node.scan_interface->kind == ScanInterfaceKind::kClient;
            });
    for (size_t id = 0; id < netlist.nodes.size(); ++id) {
      const auto& node = netlist.nodes[id];
      if (!node.read_callback && !node.write_callback) continue;
      callback_storage_[id] = true;
      callback_unknown_.emplace_back(node.name, std::string(node.Width(), 'x'));
    }
    MaskCallbackStorage();
  }

  const ReadbackUpdates* Read(const std::vector<Operation>& operations,
                              bool clear_status, bool publish) {
    if (error_) std::rethrow_exception(error_);
    try {
      // Reject unsupported histories before exposing any observation delta.
      auto callback_pending = callback_pending_;
      for (size_t i = cursor_; i < operations.size(); ++i) {
        const auto& op = operations[i];
        for (const auto consumed : op.callback_consumed) {
          const auto pending =
              callback_pending.find(operations.at(consumed).call_stack);
          if (pending != callback_pending.end()) {
            pending->second.erase(consumed);
            if (pending->second.empty()) callback_pending.erase(pending);
          }
        }
        if (op.callback_prerequisite && !planning_) {
          throw std::runtime_error(
              op.origin +
              ": modeled readback excludes automatic callback histories");
        }
        if (physical_ && op.callback_execution) {
          throw std::runtime_error(
              op.origin + ": modeled readback excludes callback histories");
        }
        if (op.callback) {
          const auto target = ResolvePdlTarget(netlist_, op.arguments.at(0));
          if (std::any_of(target.bits.begin(), target.bits.end(),
                          [&](const auto& bit) {
                            const auto& node = netlist_.nodes.at(bit.node);
                            return (op.command == "iRead" ? node.read_callback
                                                          : node.write_callback)
                                .has_value();
                          })) {
            if (op.callback_consumed.empty()) {
              callback_pending[op.call_stack].insert(i);
            }
          }
          continue;
        }
        if (op.command == "iReset") callback_pending.clear();
        if (op.command == "iApply" && callback_pending.count(op.call_stack)) {
          if (!op.callback_execution) {
            throw std::runtime_error(
                op.origin + ": modeled readback excludes callback histories");
          }
          callback_pending.erase(op.call_stack);
        }
        if (!callback_unknown_.empty() &&
            (op.command == "iWrite" || op.command == "iRead")) {
          const auto target = ResolvePdlTarget(netlist_, op.arguments.at(0));
          if (std::any_of(target.bits.begin(), target.bits.end(),
                          [&](const auto& bit) {
                            return callback_storage_.at(bit.node);
                          })) {
            throw std::runtime_error(
                op.origin +
                ": modeled readback excludes unexpanded callback access");
          }
        }
        const bool merge_marker =
            op.command == "iMerge" && op.arguments.size() == 1 &&
            (op.arguments[0] == "-begin" || op.arguments[0] == "-end") &&
            !physical_;
        if (!merge_marker && op.command != "iWrite" && op.command != "iRead" &&
            op.command != "iApply" && op.command != "iState" &&
            op.command != "iScan" && op.command != "iReset" &&
            op.command != "iTake" && op.command != "iRelease" &&
            op.command != "iNote" && op.command != "iGetStatus") {
          throw std::runtime_error(op.origin +
                                   ": modeled readback does not support " +
                                   op.command + " histories");
        }
      }
      callback_pending_ = std::move(callback_pending);
      if (published_) {
        updates_.bits.clear();
        updates_.scans.clear();
        positions_.clear();
        raw_positions_.clear();
      }
      published_ = publish;
      for (; cursor_ < operations.size(); ++cursor_) {
        const auto& op = operations[cursor_];
        if (op.callback) continue;
        try {
          Process(op);
        } catch (const std::exception& error) {
          throw std::runtime_error(
              op.origin + ": " +
              (op.callback_preserved.empty()
                   ? ""
                   : "while preserving callback caller goals: ") +
              error.what());
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

  bool CanRestoreChildHistory(const Operation& note) {
    return last_root_ &&
           scheduler_->ModeledRawHistory(note, last_root_->first).has_value();
  }

 private:
  void MaskCallbackStorage() {
    for (const auto& [name, bits] : callback_unknown_) {
      scheduler_->Assume(name, bits);
    }
  }
  char Mismatch(char expected, char actual) {
    const char mismatch = expected == 'x'      ? '0'
                          : actual == 'x'      ? 'x'
                          : actual == expected ? '0'
                                               : '1';
    if (mismatch == 'x') failures_.reset();
    if (mismatch == '1' && failures_) ++*failures_;
    return mismatch;
  }
  void ObserveRaw(const Operation& scan, const std::string& actual) {
    const auto fields = ReadScanFields(scan);
    const auto expected = scan.arguments[fields.expected].substr(2);
    if (actual.size() != expected.size() ||
        actual.find_first_not_of("01x") != std::string::npos) {
      throw std::runtime_error("invalid modeled raw observation");
    }
    std::string mismatches;
    mismatches.reserve(actual.size());
    for (size_t i = 0; i < actual.size(); ++i) {
      mismatches += Mismatch(expected[i], actual[i]);
    }
    const auto node = netlist_.node_ids.find(scan.arguments[fields.target]);
    if (modeled_children_ && !fields.instruction &&
        node != netlist_.node_ids.end() &&
        !netlist_.nodes[node->second].scope) {
      last_root_ = std::make_pair(scan, actual);
    }
    if (scan.raw_history_omitted) return;
    StoreRaw({scan.arguments[fields.target], fields.chain, actual,
              std::move(mismatches)});
  }
  void StoreRaw(ScanReadbackUpdate update) {
    const auto key = std::make_pair(update.target, update.chain);
    const auto [position, inserted] =
        raw_positions_.emplace(key, updates_.scans.size());
    if (inserted) {
      updates_.scans.push_back(std::move(update));
    } else {
      updates_.scans[position->second] = std::move(update);
    }
  }
  void Observe(const std::vector<BitReference>& bits,
               const std::string& expected, const std::string& actual) {
    for (size_t i = 0; i < bits.size(); ++i) {
      const char mismatch = Mismatch(expected[i], actual[i]);
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
    if (op.command != "iNote") last_root_.reset();
    reservations_.Enter(op.call_stack);
    if (op.command == "iMerge") return;
    const auto& args = op.arguments;
    if (op.command == "iGetStatus") {
      if (args.size() != 2 || args[0] != "-clear" ||
          !ReadStatusClearNote(
              {"iNote", {"-comment", StatusClearNoteText(args[1])}, "", {}})) {
        throw std::runtime_error("invalid internal status-clear event");
      }
      if (failures_) failures_ = 0;
      return;
    }
    if (opaque_) {
      if (op.command == "iScan") {
        opaque_->Add(context_, op);
      } else if (op.command == "iApply") {
        const auto requested = opaque_->Pending();
        opaque_->Apply(op);
        for (const auto& [name, scan] : requested) {
          const auto fields = ReadScanFields(scan);
          ObserveRaw(scan,
                     std::string(scan.arguments[fields.input].size() - 2, 'x'));
        }
      } else if (op.command == "iReset") {
        opaque_->Reset(op);
      } else if (op.command != "iNote") {
        throw std::runtime_error("unsupported operation with opaque scans: " +
                                 op.command);
      }
      return;
    }
    if (op.command == "iNote") {
      if (op.modeled_raw_history && last_root_) {
        const auto projection =
            scheduler_->ModeledRawHistory(op, last_root_->first);
        if (projection) {
          auto read = last_root_->second.substr(projection->offset,
                                                projection->expected.size());
          std::string mismatches;
          mismatches.reserve(read.size());
          for (size_t i = 0; i < read.size(); ++i) {
            if (projection->inverted && read[i] != 'x') {
              read[i] = read[i] == '1' ? '0' : '1';
            }
            const char expected = projection->expected[i];
            mismatches += expected == 'x'       ? '0'
                          : read[i] == 'x'      ? 'x'
                          : read[i] == expected ? '0'
                                                : '1';
          }
          StoreRaw(
              {projection->target, "", std::move(read), std::move(mismatches)});
          return;
        }
      }
      last_root_.reset();
      scheduler_->RestoreOpaqueScanNote(op);
      if (auto actual = scheduler_->OpaqueRawHistory(op)) {
        StoreRaw(std::move(*actual));
      }
    } else if (op.command == "iScan") {
      auto scan = scheduler_->NormalizeScan(op, true);
      const auto fields = ReadScanFields(scan);
      const auto key = std::make_tuple(
          fields.instruction, scan.arguments[fields.target], fields.chain);
      const size_t previous =
          raw_pending_.count(key)
              ? raw_pending_.at(key)
                        .arguments[ReadScanFields(raw_pending_.at(key)).input]
                        .size() -
                    2
              : 0;
      const size_t bits = raw_pending_bits_ - previous +
                          scan.arguments[fields.input].size() - 2;
      if (bits > 1024 * 1024) {
        throw std::runtime_error("raw scan queued payload resource limit");
      }
      raw_pending_[key] = std::move(scan);
      raw_pending_bits_ = bits;
      return;
    }
    if (op.command == "iWrite" || op.command == "iRead") {
      const auto target = ResolvePdlTarget(netlist_, args.at(0));
      ValidatePdlAccessTarget(netlist_, target, op.command == "iRead");
      const auto bits = args.size() == 2
                            ? PdlDataValue(context_, netlist_, target, args[1])
                            : std::string(target.bits.size(), 'x');
      pending_.Add(op, target, bits);
    } else if (op.command == "iApply") {
      const auto caller_values = reservations_.CallbackValues(op, pending_);
      const auto& preserved =
          caller_values ? *caller_values : reservations_.Values();
      if (!op.callback_preserved.empty() && !raw_pending_.empty()) {
        throw std::runtime_error(
            "raw scans with callback caller goals are unsupported");
      }
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
      const bool together = ValidatePdlApplyOptions(args);
      if (together && !raw_pending_.empty() && !pending_.Empty()) {
        throw std::runtime_error(
            "mixed raw preparation cannot use iApply -together");
      }
      if (raw_pending_.empty() || !pending_.Empty()) {
        if (planning_) {
          const auto steps = planning_->Apply(
              accesses, together, preserved, true, false,
              [](const auto&, const auto&) {}, op.callback_preserved);
          scheduler_->ReplayPrimary(
              accesses, steps, preserved,
              bits.empty() ? ScanScheduler::PioObservation{} : observe,
              op.callback_preserved);
        } else {
          scheduler_->Apply(
              accesses, raw_pending_.empty() && together, preserved, true,
              physical_,
              bits.empty() ? ScanScheduler::PioObservation{} : observe,
              op.callback_preserved);
        }
      }
      if (actual.find('?') != std::string::npos) {
        throw std::runtime_error(
            "modeled apply did not observe every read bit");
      }
      reservations_.Assigned(accesses, *scheduler_);
      Observe(bits, expected, actual);
      pending_.Clear();
      if (!raw_pending_.empty()) {
        std::vector<Operation> scans;
        for (const auto& [key, scan] : raw_pending_) scans.push_back(scan);
        std::set<size_t> assigned;
        std::vector<ScanStep> preparations;
        scheduler_->ApplyRawScans(
            scans, preserved, preserved.empty() ? nullptr : &assigned,
            &preparations, [&](const auto& scan, const auto& actual) {
              ObserveRaw(scan, actual);
            });
        reservations_.AssignedScans(assigned, *scheduler_);
        raw_pending_.clear();
        raw_pending_bits_ = 0;
      }
    } else if (op.command == "iReset") {
      scheduler_->Reset(!args.empty(), physical_);
      if (planning_) planning_->Reset(!args.empty(), false);
      MaskCallbackStorage();
      reservations_.Reset();
      pending_.Clear();
      raw_pending_.clear();
      raw_pending_bits_ = 0;
    } else if (op.command == "iState") {
      const auto target = ResolvePdlTarget(netlist_, args.at(0));
      auto bits = PdlNumber(context_, args.at(1), target.bits.size());
      if (args.size() != 3 || args[2] == "-LastWrittenValue") {
        if (planning_) {
          planning_->Assume(
              args[0], bits,
              reservations_.CallbackConstraints(op, pending_, planning_.get()));
        }
        for (size_t i = 0; i < target.bits.size(); ++i) {
          if (callback_storage_.at(target.bits[i].node)) bits[i] = 'x';
        }
        scheduler_->Assume(
            args[0], bits,
            reservations_.CallbackConstraints(op, pending_, scheduler_.get()));
        pending_.Assume(target, bits);
        reservations_.Assigned({op}, *scheduler_);
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

  std::optional<Netlist> observation_netlist_;
  const Netlist& netlist_;
  bool physical_;
  std::map<std::vector<std::string>, std::set<size_t>> callback_pending_;
  std::vector<bool> callback_storage_;
  std::vector<std::pair<std::string, std::string>> callback_unknown_;
  z3::context context_;
  std::unique_ptr<ScanScheduler> scheduler_;
  std::unique_ptr<ScanScheduler> planning_;
  std::unique_ptr<OpaqueScanQueue> opaque_;
  std::map<std::tuple<bool, std::string, std::string>, Operation> raw_pending_;
  size_t raw_pending_bits_ = 0;
  ResourceReservations reservations_;
  QueuedAccesses pending_;
  size_t cursor_ = 0;
  std::optional<uint64_t> failures_{0};
  ReadbackUpdates updates_;
  bool modeled_children_ = false;
  std::optional<std::pair<Operation, std::string>> last_root_;
  bool published_ = true;
  std::map<std::pair<size_t, int64_t>, size_t> positions_;
  std::map<std::pair<std::string, std::string>, size_t> raw_positions_;
  std::exception_ptr error_;
};
}  // namespace

Runtime::Readback MakeModeledReadback(const Netlist& netlist) {
  return MakeModeledReadback(netlist, false);
}
Runtime::Readback MakeModeledReadback(const Netlist& netlist, bool physical) {
  return MakeModeledReadback(netlist, physical, nullptr);
}
Runtime::Readback MakeModeledReadback(
    const Netlist& netlist, bool physical,
    Runtime::StatusReadback* status_readback,
    Runtime::HistoryReadback* history_readback) {
  auto model = std::make_shared<std::unique_ptr<ModelReadback>>();
  const auto read = [&netlist, physical, model](const auto& operations,
                                                bool clear_status,
                                                bool publish) {
    if (!*model) *model = std::make_unique<ModelReadback>(netlist, physical);
    return (*model)->Read(operations, clear_status, publish);
  };
  if (status_readback) {
    *status_readback = [read](const auto& operations) {
      return read(operations, false, false)->failures;
    };
  }
  if (history_readback) {
    *history_readback = [read, model](const auto& operations,
                                      const auto& note) {
      read(operations, false, false);
      return (*model)->CanRestoreChildHistory(note);
    };
  }
  return [read](const auto& operations, bool clear_status) {
    return read(operations, clear_status, true);
  };
}
}  // namespace pattern
