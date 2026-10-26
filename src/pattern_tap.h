// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_TAP_H_
#define ICL_TOOLSETS_SRC_PATTERN_TAP_H_

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "pattern_access_link.h"
#include "pattern_netlist.h"

namespace pattern {
enum class TapState {
  kTestLogicReset,
  kRunTestIdle,
  kSelectDr,
  kCaptureDr,
  kShiftDr,
  kExit1Dr,
  kPauseDr,
  kExit2Dr,
  kUpdateDr,
  kSelectIr,
  kCaptureIr,
  kShiftIr,
  kExit1Ir,
  kPauseIr,
  kExit2Ir,
  kUpdateIr,
};

constexpr uint16_t TapStateBit(TapState state) {
  return uint16_t{1} << static_cast<unsigned>(state);
}

struct TapActions {
  char capture_ir = '0';
  char shift_ir = '0';
  char update_ir = '0';
  char capture_dr = '0';
  char shift_dr = '0';
  char update_dr = '0';
  char reset_ir = '0';
};

// No observed power-up reset is assumed. Unknown inputs retain possible states.
// Call once per actual edge; returned actions apply only to that event. Inputs
// are normalized 0/1/x. This controller does not hold shift/latch register
// data.
class TapController {
 public:
  TapActions SetTrst(char value);
  TapActions RisingEdge(char tms);
  TapActions FallingEdge();
  uint16_t PossibleStates() const { return states_; }
  char TdoEnabled() const { return tdo_enabled_; }
  char IrSelect() const { return ir_select_; }

 private:
  uint16_t states_ = 0xffff;
  char trst_ = 'x';
  char tdo_enabled_ = 'x';
  char ir_select_ = 'x';
};

// Executes IR events only; DR actions are returned for the surrounding runtime.
// Codes are MSB-first 0/1/x patterns, retaining reset encoding alternatives.
// State joins conservatively lose correlations; no hardware reset is assumed.
class TapInstructionRegister {
 public:
  explicit TapInstructionRegister(const BsdlTap& bsdl);
  TapActions SetTrst(char value);
  TapActions RisingEdge(char tms, char tdi);
  TapActions FallingEdge();
  const TapController& Controller() const { return controller_; }
  const std::vector<std::string>& InstructionCodes() const { return codes_; }
  std::string ShiftBits() const;
  char ShiftOut() const;

 private:
  std::optional<std::vector<std::string>> NextInstructions(
      const TapActions& actions) const;
  void Shift(const TapActions& actions, char tdi);
  TapController controller_;
  std::string origin_, capture_, shift_bits_;
  std::vector<std::string> reset_codes_, codes_;
  size_t shift_head_ = 0;
  bool shift_valid_ = false;
  bool has_trst_ = false;
};

struct TapDatapathModel : TapDatapathNodes {
  Netlist netlist;
};

struct TapNetwork {
  Netlist netlist;
  std::map<std::string, TapDatapathNodes> taps;
};

// Builds an owning IR/ActiveSignal/DR datapath through the shared ICL frontend.
// ir_select and scan_returns are boundary inputs, not an implemented TAP FSM or
// connected child chains. No reset, clock timing, or initialization is implied.
// Standard register associations are inferred; other non-AccessLink
// instructions need REGISTER_ACCESS. Unknown binary opcodes route to BYPASS.
// Identification capture metadata currently supports one 32-bit pattern per
// IDCODE/USERCODE.
TapDatapathModel BuildTapDatapathModel(
    const AccessLink& link,
    const std::string& module_name = "IclPatternTapDatapath");
// Assemble TAP pins, instruction-selected serial routes and client controls.
// Flat ICL restores validated controller associations for hardware replay.
// Scheduling is separate from assembly and observed pin-event execution.
TapNetwork ElaborateTapNetwork(Design& design, const std::string& top);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_TAP_H_
