// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_tap.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "pattern_flatten.h"
#include "pattern_logic.h"
#include "text_input.h"

namespace {
void Require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}
std::string Read(const std::string& path) {
  std::ifstream input(path);
  std::string text;
  Require(input.good() && toolsets::ReadTextStream(input, &text),
          "read " + path);
  return text;
}
const std::string kDirectory =
    "tests/cases/ours/pattern_generation/access_link/";
std::string Actions(const pattern::TapActions& actions) {
  // Capture/shift/update IR, capture/shift/update DR, reset IR.
  return {actions.capture_ir, actions.shift_ir, actions.update_ir,
          actions.capture_dr, actions.shift_dr, actions.update_dr,
          actions.reset_ir};
}
void TestControllerTransitions() {
  using S = pattern::TapState;
  struct Transition {
    S state;
    const char* path;
    S low, high;
    const char* rising;
    const char* falling;
    const char* tdo;
  };
  // IEEE 1149.1-2001 Figure 6-1; paths start in Test-Logic-Reset.
  const Transition cases[] = {
      {S::kTestLogicReset, "", S::kRunTestIdle, S::kTestLogicReset, "0000000",
       "0000001", "0x"},
      {S::kRunTestIdle, "0", S::kRunTestIdle, S::kSelectDr, "0000000",
       "0000000", "0x"},
      {S::kSelectDr, "01", S::kCaptureDr, S::kSelectIr, "0000000", "0000000",
       "0x"},
      {S::kCaptureDr, "010", S::kShiftDr, S::kExit1Dr, "0001000", "0000000",
       "0x"},
      {S::kShiftDr, "0100", S::kShiftDr, S::kExit1Dr, "0000100", "0000000",
       "10"},
      {S::kExit1Dr, "0101", S::kPauseDr, S::kUpdateDr, "0000000", "0000000",
       "0x"},
      {S::kPauseDr, "01010", S::kPauseDr, S::kExit2Dr, "0000000", "0000000",
       "0x"},
      {S::kExit2Dr, "010101", S::kShiftDr, S::kUpdateDr, "0000000", "0000000",
       "0x"},
      {S::kUpdateDr, "01011", S::kRunTestIdle, S::kSelectDr, "0000000",
       "0000010", "0x"},
      {S::kSelectIr, "011", S::kCaptureIr, S::kTestLogicReset, "0000000",
       "0000000", "0x"},
      {S::kCaptureIr, "0110", S::kShiftIr, S::kExit1Ir, "1000000", "0000000",
       "0x"},
      {S::kShiftIr, "01100", S::kShiftIr, S::kExit1Ir, "0100000", "0000000",
       "11"},
      {S::kExit1Ir, "01101", S::kPauseIr, S::kUpdateIr, "0000000", "0000000",
       "0x"},
      {S::kPauseIr, "011010", S::kPauseIr, S::kExit2Ir, "0000000", "0000000",
       "0x"},
      {S::kExit2Ir, "0110101", S::kShiftIr, S::kUpdateIr, "0000000", "0000000",
       "0x"},
      {S::kUpdateIr, "011011", S::kRunTestIdle, S::kSelectDr, "0000000",
       "0010000", "0x"},
  };
  for (const auto& test : cases) {
    for (char tms : {'0', '1', 'x'}) {
      pattern::TapController controller;
      controller.SetTrst('0');
      controller.SetTrst('1');
      for (char bit : std::string(test.path)) controller.RisingEdge(bit);
      Require(controller.PossibleStates() == pattern::TapStateBit(test.state),
              "reach the independently specified starting state");
      Require(Actions(controller.FallingEdge()) == test.falling &&
                  std::string{controller.TdoEnabled(), controller.IrSelect()} ==
                      test.tdo,
              "falling actions and TDO match IEEE 1149.1 Tables 6-2 and 7-1");
      Require(
          Actions(controller.RisingEdge(tms)) == test.rising,
          "rising actions use pre-transition state even when TMS is unknown");
      const auto expected = tms == '0'   ? pattern::TapStateBit(test.low)
                            : tms == '1' ? pattern::TapStateBit(test.high)
                                         : pattern::TapStateBit(test.low) |
                                               pattern::TapStateBit(test.high);
      Require(controller.PossibleStates() == expected,
              "every known/unknown TMS transition matches Figure 6-1");
    }
  }
}
void TestControllerEdgesAndReset() {
  using S = pattern::TapState;
  pattern::TapController controller;
  Require(controller.PossibleStates() == 0xffff &&
              controller.TdoEnabled() == 'x' && controller.IrSelect() == 'x',
          "construction does not assume an observed hardware reset");
  controller.SetTrst('1');
  for (int i = 0; i < 4; ++i) controller.RisingEdge('1');
  Require(
      controller.PossibleStates() != pattern::TapStateBit(S::kTestLogicReset),
      "four high TMS clocks do not reset every possible starting state");
  controller.RisingEdge('1');
  Require(
      controller.PossibleStates() == pattern::TapStateBit(S::kTestLogicReset),
      "five high TMS clocks synchronize unknown state");
  Require(controller.FallingEdge().reset_ir == '1',
          "synchronous instruction reset occurs on falling TCK in reset");
  Require(controller.TdoEnabled() == '0', "reset disables TDO at falling TCK");

  for (bool ir : {false, true}) {
    controller.SetTrst('0');
    controller.SetTrst('1');
    const std::string prefix = ir ? "0110" : "010";
    for (char bit : prefix) {
      const auto actions = controller.RisingEdge(bit);
      Require(actions.capture_ir == '0' && actions.capture_dr == '0' &&
                  actions.shift_ir == '0' && actions.shift_dr == '0' &&
                  actions.update_ir == '0' && actions.update_dr == '0' &&
                  actions.reset_ir == '0',
              "entering capture does not capture prematurely");
      controller.FallingEdge();
    }
    auto actions = controller.RisingEdge('0');
    Require(actions.capture_ir == (ir ? '1' : '0') &&
                actions.capture_dr == (ir ? '0' : '1') &&
                actions.shift_ir == '0' && actions.shift_dr == '0',
            "capture occurs on the rising edge leaving Capture");
    Require(controller.TdoEnabled() == '0',
            "entry into Shift does not enable TDO until falling edge");
    controller.FallingEdge();
    Require(controller.TdoEnabled() == '1' &&
                controller.IrSelect() == (ir ? '1' : '0'),
            "falling TCK selects IR or DR and enables TDO in Shift");
    actions = controller.RisingEdge('1');
    Require(
        actions.shift_ir == (ir ? '1' : '0') &&
            actions.shift_dr == (ir ? '0' : '1') &&
            controller.TdoEnabled() == '1',
        "the exit edge still shifts; TDO remains enabled until falling TCK");
    controller.FallingEdge();
    Require(controller.TdoEnabled() == '0',
            "Exit1 disables TDO on falling edge");
    actions = controller.RisingEdge('1');
    Require(actions.update_ir == '0' && actions.update_dr == '0',
            "entering Update does not latch on rising TCK");
    actions = controller.FallingEdge();
    Require(actions.update_ir == (ir ? '1' : '0') &&
                actions.update_dr == (ir ? '0' : '1') &&
                actions.capture_ir == '0' && actions.capture_dr == '0' &&
                actions.shift_ir == '0' && actions.shift_dr == '0',
            "only the selected update action fires on falling TCK");
  }
  Require(controller.SetTrst('0').reset_ir == '1' &&
              controller.PossibleStates() ==
                  pattern::TapStateBit(S::kTestLogicReset),
          "TRST asserts instruction reset asynchronously");
  Require(controller.RisingEdge('0').reset_ir == '1' &&
              controller.PossibleStates() ==
                  pattern::TapStateBit(S::kTestLogicReset),
          "held TRST prevents state advance");
  Require(controller.SetTrst('1').reset_ir == '0',
          "TRST release does not itself perform a synchronous reset");
  controller.RisingEdge('0');
  Require(controller.SetTrst('x').reset_ir == 'x' &&
              controller.PossibleStates() ==
                  (pattern::TapStateBit(S::kRunTestIdle) |
                   pattern::TapStateBit(S::kTestLogicReset)),
          "unknown reset retains both reset and non-reset possibilities");
  Require(controller.FallingEdge().reset_ir == 'x', "ambiguous reset action");
  controller.SetTrst('1');
  controller.RisingEdge('0');
  Require(controller.PossibleStates() == pattern::TapStateBit(S::kRunTestIdle),
          "uncertainty may converge after reset release");
  controller.RisingEdge('1');
  controller.RisingEdge('x');
  Require(controller.RisingEdge('0').capture_dr == 'x',
          "unknown TMS yields a conditional capture action");
  controller.FallingEdge();
  Require(controller.TdoEnabled() == 'x' && controller.IrSelect() == 'x',
          "TDO controls conservatively retain uncertainty");
  const auto states = controller.PossibleStates();
  for (char invalid : {'X', 'z', '2', '\0'}) {
    for (bool reset : {false, true}) {
      bool rejected = false;
      try {
        if (reset)
          controller.SetTrst(invalid);
        else
          controller.RisingEdge(invalid);
      } catch (const std::invalid_argument&) {
        rejected = true;
      }
      Require(rejected && controller.PossibleStates() == states &&
                  controller.TdoEnabled() == 'x' &&
                  controller.IrSelect() == 'x',
              "invalid input is rejected without state or output mutation");
    }
  }
  controller.SetTrst('0');
  controller.SetTrst('1');
  for (char bit : std::string("01100")) controller.RisingEdge(bit);
  controller.FallingEdge();
  controller.SetTrst('x');
  Require(Actions(controller.RisingEdge('0')) == "0x0000x",
          "unknown TRST may suppress an IR shift and asynchronously reset IR");
  controller.SetTrst('0');
  Require(controller.TdoEnabled() == '1' && controller.IrSelect() == '1',
          "asynchronous reset does not change falling-edge TDO controls early");
  Require(Actions(controller.FallingEdge()) == "0000001" &&
              controller.TdoEnabled() == '0',
          "held asynchronous reset prevents updates and disables TDO on "
          "falling TCK");
}
std::string Replace(std::string text, const std::string& from,
                    const std::string& to) {
  const auto at = text.find(from);
  Require(at != std::string::npos, "replacement exists: " + from);
  text.replace(at, from.size(), to);
  return text;
}
pattern::BsdlTap InstructionMetadata() {
  return pattern::ReadBsdlTap("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                              "AccessProbe");
}
void InstructionClock(pattern::TapInstructionRegister* reg, char tms,
                      char tdi = 'x') {
  reg->RisingEdge(tms, tdi);
  reg->FallingEdge();
}
void LoadInstruction(pattern::TapInstructionRegister* reg,
                     const std::string& bits) {
  // Start in Run-Test/Idle; enter Capture-IR, capture, then shift LSB first.
  for (char tms : std::string("1100")) InstructionClock(reg, tms);
  for (size_t offset = 0; offset < bits.size(); ++offset) {
    InstructionClock(reg, offset + 1 == bits.size() ? '1' : '0',
                     bits[bits.size() - 1 - offset]);
  }
  InstructionClock(reg, '1');
  InstructionClock(reg, '0');
}
void TestInstructionExecution() {
  pattern::TapInstructionRegister reg(InstructionMetadata());
  Require(reg.ShiftBits() == "xxxx" && reg.ShiftOut() == 'x' &&
              reg.InstructionCodes() == std::vector<std::string>{"xxxx"},
          "instruction construction leaves both stages unknown");
  reg.SetTrst('0');
  Require(reg.InstructionCodes() == std::vector<std::string>{"0010"} &&
              reg.ShiftBits() == "xxxx",
          "TRST initializes only the active instruction");
  reg.SetTrst('1');
  for (char tms : std::string("0110")) InstructionClock(&reg, tms);
  Require(reg.ShiftBits() == "xxxx", "entry into Capture-IR does not capture");
  InstructionClock(&reg, '0');
  Require(reg.ShiftBits() == "0001" && reg.ShiftOut() == '1',
          "Capture-IR loads the BSDL capture pattern on the next rising edge");
  const std::string shifted[] = {"0000", "1000", "0100", "1010"};
  for (size_t i = 0; i < 4; ++i) {
    InstructionClock(&reg, i == 3 ? '1' : '0', "0101"[i]);
    Require(reg.ShiftBits() == shifted[i] &&
                reg.InstructionCodes() == std::vector<std::string>{"0010"},
            "LSB-first shifts do not change the active instruction latch");
  }
  reg.RisingEdge('1', 'x');
  Require(reg.InstructionCodes() == std::vector<std::string>{"0010"},
          "entering Update-IR does not latch at the rising edge");
  reg.FallingEdge();
  Require(
      reg.InstructionCodes() == std::vector<std::string>{"1010"},
      "falling Update-IR transfers the shift stage into the instruction latch");
  InstructionClock(&reg, '0');
  LoadInstruction(&reg, "0101");
  Require(reg.InstructionCodes() == std::vector<std::string>{"0101"},
          "a subsequent scan captures and replaces the prior instruction");
  reg.SetTrst('0');
  Require(reg.InstructionCodes() == std::vector<std::string>{"0010"} &&
              reg.ShiftBits() == "xxxx",
          "reset discards undefined shift-stage knowledge");
}

void TestInstructionResetAlternatives() {
  auto bsdl = InstructionMetadata();
  bsdl.opcodes["IDCODE"] = {"1010", "0010", "1010", "10x0"};
  pattern::TapInstructionRegister reg(bsdl);
  bsdl.capture = "1111";
  bsdl.opcodes.clear();
  reg.SetTrst('0');
  Require(
      reg.InstructionCodes() ==
          std::vector<std::string>({"0010", "1010", "10x0"}),
      "owned reset alternatives retain masks instead of inventing one opcode");
  reg.SetTrst('1');
  InstructionClock(&reg, '0');
  LoadInstruction(&reg, "0101");
  reg.SetTrst('x');
  Require(
      reg.InstructionCodes() ==
          std::vector<std::string>({"0010", "0101", "1010", "10x0"}),
      "unknown TRST retains both active instruction and reset alternatives");
  reg.SetTrst('1');
  for (int i = 0; i < 5; ++i) reg.RisingEdge('1', 'x');
  Require(reg.InstructionCodes().size() == 4,
          "five rising TMS clocks do not latch synchronous reset early");
  reg.FallingEdge();
  Require(reg.InstructionCodes().size() == 3,
          "synchronous reset selects only reset alternatives on falling TCK");
  auto no_id = InstructionMetadata();
  no_id.idcode.reset();
  no_id.opcodes.erase("IDCODE");
  pattern::TapInstructionRegister bypass(no_id);
  bypass.SetTrst('0');
  Require(bypass.InstructionCodes() == std::vector<std::string>{"1111"},
          "a device without IDCODE resets to BYPASS");
}
void TestInstructionWithoutTrst() {
  auto metadata = InstructionMetadata();
  metadata.ports.erase("tap_scan_reset");
  pattern::TapInstructionRegister reg(metadata);
  Require(reg.Controller().PossibleStates() == 0xffff &&
              reg.InstructionCodes() == std::vector<std::string>{"xxxx"},
          "an absent TRST pin must not imply a power-up reset");
  for (int i = 0; i < 5; ++i) InstructionClock(&reg, '1');
  Require(reg.InstructionCodes() == std::vector<std::string>{"0010"},
          "TMS-only reset works without the optional TRST pin");
  InstructionClock(&reg, '0');
  Require(reg.Controller().PossibleStates() ==
              pattern::TapStateBit(pattern::TapState::kRunTestIdle),
          "absent TRST cannot hold the controller in reset after TMS reset");
  LoadInstruction(&reg, "0011");
  Require(reg.InstructionCodes() == std::vector<std::string>{"0011"},
          "IR scans work when the BSDL TAP has no TRST pin");
  reg.SetTrst('1');
  for (char value : {'0', 'x'}) {
    bool rejected = false;
    try {
      reg.SetTrst(value);
    } catch (const std::runtime_error& error) {
      rejected = std::string(error.what()).find("no TRST") != std::string::npos;
    }
    Require(
        rejected && reg.InstructionCodes() == std::vector<std::string>{"0011"},
        "a nonexistent TRST pin cannot asynchronously reset the device");
  }
}
void TestInstructionUnknownsAndValidation() {
  auto metadata = InstructionMetadata();
  // An unknown initial instruction already includes every reset encoding.
  metadata.instruction_width = 1024 * 1024;
  metadata.capture = std::string(metadata.instruction_width - 2, 'x') + "01";
  metadata.opcodes["IDCODE"] = {std::string(metadata.instruction_width, '0')};
  pattern::TapInstructionRegister wide(metadata);
  wide.RisingEdge('0', 'x');
  Require(
      wide.InstructionCodes() == std::vector<std::string>{std::string(
                                     metadata.instruction_width, 'x')},
      "unknown reset must not expand an already universal instruction state");
  pattern::TapInstructionRegister reg(InstructionMetadata());
  reg.SetTrst('0');
  reg.SetTrst('1');
  InstructionClock(&reg, '0');
  for (char tms : std::string("1100")) InstructionClock(&reg, tms);
  InstructionClock(&reg, 'x', '1');
  Require(reg.ShiftBits() == "1000", "unknown exit decision still shifts once");
  InstructionClock(&reg, '0', '0');
  Require(reg.ShiftBits() == "xx00",
          "uncertain Shift/Pause path joins hold and shift bit values");
  for (char invalid : {'z', 'X', '2'}) {
    for (int input = 0; input < 3; ++input) {
      const auto states = reg.Controller().PossibleStates();
      const auto shift = reg.ShiftBits();
      const auto codes = reg.InstructionCodes();
      bool rejected = false;
      try {
        if (input == 0) reg.SetTrst(invalid);
        if (input == 1) reg.RisingEdge(invalid, '0');
        if (input == 2) reg.RisingEdge('0', invalid);
      } catch (const std::invalid_argument&) {
        rejected = true;
      }
      Require(rejected && reg.Controller().PossibleStates() == states &&
                  reg.ShiftBits() == shift && reg.InstructionCodes() == codes,
              "invalid pin values cannot partly advance the IR model");
    }
  }
  const auto reject = [](const pattern::BsdlTap& bsdl,
                         const std::string& cause) {
    bool rejected = false;
    try {
      pattern::TapInstructionRegister invalid(bsdl);
    } catch (const std::runtime_error& error) {
      const std::string message = error.what();
      rejected = message.find(bsdl.origin) == 0 &&
                 message.find(cause) != std::string::npos;
    }
    Require(rejected,
            "invalid BSDL IR metadata reports its origin and cause: " + cause);
  };
  for (size_t width : {size_t{0}, size_t{1}, size_t{1024 * 1024 + 1}}) {
    auto bad = InstructionMetadata();
    bad.instruction_width = width;
    reject(bad, "width/capture");
  }
  for (const auto* capture : {"01", "00z1", "0011"}) {
    auto bad = InstructionMetadata();
    bad.capture = capture;
    reject(bad, "width/capture");
  }
  auto bad = InstructionMetadata();
  bad.idcode.reset();
  reject(bad, "IDCODE_REGISTER");
  bad = InstructionMetadata();
  bad.opcodes.erase("IDCODE");
  reject(bad, "missing reset");
  bad.opcodes["IDCODE"] = {};
  reject(bad, "missing reset");
  for (const auto* code : {"01", "00z0"}) {
    bad.opcodes["IDCODE"] = {code};
    reject(bad, "invalid reset");
  }
  bad.opcodes["IDCODE"].assign(262145, "0010");
  reject(bad, "work limit");
}

void TestInstructionShiftScale() {
  auto metadata = InstructionMetadata();
  constexpr size_t kWidth = 65537;
  constexpr size_t kEdges = 262151;
  metadata.instruction_width = kWidth;
  metadata.capture = std::string(kWidth - 2, 'x') + "01";
  metadata.opcodes["IDCODE"] = {std::string(kWidth, '0')};
  pattern::TapInstructionRegister reg(metadata);
  reg.SetTrst('0');
  reg.SetTrst('1');
  for (char tms : std::string("01100")) InstructionClock(&reg, tms);
  const auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < kEdges; ++i)
    InstructionClock(&reg, '0', i % 2 ? '1' : '0');
  const auto seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
          .count();
  std::string expected(kWidth, 'x');
  for (size_t i = 0; i < kWidth; ++i)
    expected[i] = (kEdges - 1 - i) % 2 ? '1' : '0';
  Require(reg.ShiftBits() == expected && reg.ShiftOut() == expected.back(),
          "large repeated shifts wrap without reversing bit order");
  Require(reg.InstructionCodes() ==
              std::vector<std::string>{std::string(kWidth, '0')},
          "large shifting does not mutate the active instruction latch");
  std::cout << "TAP IR: width=" << kWidth << " edges=" << kEdges
            << " seconds=" << seconds << '\n';
}
void TestInstructionUncertainUpdate() {
  pattern::TapInstructionRegister reg(InstructionMetadata());
  reg.SetTrst('0');
  reg.SetTrst('1');
  for (char tms : std::string("01100")) InstructionClock(&reg, tms);
  for (char tms : std::string("0001")) InstructionClock(&reg, tms, '1');
  reg.RisingEdge('x', 'x');
  Require(reg.InstructionCodes() == std::vector<std::string>{"0010"},
          "uncertain entry into Update-IR cannot latch early");
  reg.FallingEdge();
  Require(reg.InstructionCodes() == std::vector<std::string>({"0010", "1111"}),
          "uncertain Update-IR retains complete old and new code alternatives");
  reg.SetTrst('x');
  reg.FallingEdge();
  Require(
      reg.InstructionCodes() == std::vector<std::string>{"xxxx"},
      "unknown shift-stage update subsumes narrower instruction possibilities");
}

void TestInstructionStateLimits() {
  auto metadata = InstructionMetadata();
  metadata.instruction_width = 11;
  metadata.capture = "xxxxxxxxx01";
  auto& alternatives = metadata.opcodes["IDCODE"];
  alternatives.clear();
  for (size_t i = 0; i < 65537; ++i) {
    size_t code = i;
    std::string bits(11, '0');
    for (char& bit : bits) {
      bit = "01x"[code % 3];
      code /= 3;
    }
    alternatives.push_back(std::move(bits));
  }
  bool rejected = false;
  try {
    pattern::TapInstructionRegister excessive(metadata);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("instruction state limit") !=
               std::string::npos;
  }
  Require(rejected, "distinct reset alternatives have a bounded entry count");

  constexpr size_t kWidth = 65536;
  metadata.instruction_width = kWidth;
  metadata.capture = std::string(kWidth - 2, 'x') + "01";
  alternatives.clear();
  for (size_t i = 0; i < 16; ++i) {
    std::string bits(kWidth, '0');
    for (size_t bit = 0; bit < 4; ++bit) bits[bit] = (i >> bit) & 1 ? '1' : '0';
    alternatives.push_back(std::move(bits));
  }
  pattern::TapInstructionRegister reg(metadata);
  reg.SetTrst('0');
  reg.SetTrst('1');
  InstructionClock(&reg, '0');
  LoadInstruction(&reg, std::string(kWidth, '1'));
  const auto states = reg.Controller().PossibleStates();
  const auto shift = reg.ShiftBits();
  const auto codes = reg.InstructionCodes();
  rejected = false;
  try {
    reg.SetTrst('x');
  } catch (const std::runtime_error& error) {
    const std::string message = error.what();
    rejected = message.find(metadata.origin) == 0 &&
               message.find("instruction state limit") != std::string::npos;
  }
  Require(rejected && reg.Controller().PossibleStates() == states &&
              reg.ShiftBits() == shift && reg.InstructionCodes() == codes,
          "runtime union overflow preserves controller and register state");
  InstructionClock(&reg, '0');
  Require(reg.Controller().PossibleStates() ==
              pattern::TapStateBit(pattern::TapState::kRunTestIdle),
          "failed unknown TRST did not change the stored reset input");
}
pattern::TapDatapathModel Build(const std::string& icl,
                                const std::string& bsdl) {
  pattern::Design design;
  design.AddIcl("probe.icl", icl);
  design.AddBsdl("probe.bsdl", bsdl, "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  return pattern::BuildTapDatapathModel(links[0]);
}
std::string UserRegisterBsdl() {
  return Replace(
      Replace(Read(kDirectory + "simple.bsdl"), "EXTEST (0000), SAMPLE (0001)",
              "USER_ONE (0000), USER_TWO (0001)"),
      "BOUNDARY (EXTEST, SAMPLE)", "USER[9] (USER_ONE, USER_TWO)");
}
char ReadBit(const pattern::TapDatapathModel& model,
             const pattern::BitReference& bit, const std::string& instruction) {
  if (bit.node == std::numeric_limits<size_t>::max()) return bit.constant;
  if (bit.node == model.instruction_register) {
    return instruction.at(instruction.size() - 1 - bit.index);
  }
  const auto& node = model.netlist.nodes.at(bit.node);
  if (node.model == pattern::ModelKind::kLogic) {
    return pattern::EvaluateLogic(node,
                                  [&](const pattern::BitReference& child) {
                                    return ReadBit(model, child, instruction);
                                  });
  }
  return ReadBit(model,
                 node.inputs.at("Source").at(node.left > node.right
                                                 ? node.left - bit.index
                                                 : bit.index - node.left),
                 instruction);
}
void TestSharedSignalsAndMasks() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; common; }");
  icl = Replace(icl, "IJTAG_OFF {", "IJTAG_OFF { ActiveSignals { common; }");
  const auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (0011)",
                            "IJTAG_ON (0011, 10XX, 0110)");
  const auto model = Build(icl, bsdl);
  for (int value = 0; value < 16; ++value) {
    std::string bits(4, '0');
    for (size_t i = 0; i < bits.size(); ++i) bits[3 - i] += (value >> i) & 1;
    const bool on = value == 3 || value == 6 || (value >= 8 && value <= 11);
    const auto common = model.netlist.node_ids.at("common");
    const auto setup = model.netlist.node_ids.at("long_setup");
    Require(
        ReadBit(model, {setup, 0}, bits) == (on ? '1' : '0') &&
            ReadBit(model, {common, 0}, bits) == (on || value == 4 ? '1' : '0'),
        "mask alternatives and shared ActiveSignals follow truth table");
  }
  Require(ReadBit(model, {model.netlist.node_ids.at("long_setup"), 0},
                  "10xx") == '1',
          "opcode masks do not depend on unknown masked bits");
}
void TestInstructionDecoderIntegration() {
  const auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IDCODE (0010)",
                            "IDCODE (0010, 1010)");
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  pattern::TapInstructionRegister reg(
      pattern::ReadBsdlTap("probe.bsdl", bsdl, "AccessProbe"));
  const auto signal = model.netlist.node_ids.at("long_setup");
  reg.SetTrst('0');
  for (const auto& code : reg.InstructionCodes()) {
    Require(ReadBit(model, {signal, 0}, code) == '0',
            "every reset encoding leaves IJTAG ActiveSignal inactive");
  }
  reg.SetTrst('1');
  InstructionClock(&reg, '0');
  LoadInstruction(&reg, "0011");
  Require(
      reg.InstructionCodes().size() == 1 &&
          ReadBit(model, {signal, 0}, reg.InstructionCodes()[0]) == '1',
      "latched instruction state drives the shared AST/netlist opcode decoder");
}
void TestNamesAndMissingSignals() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; INSTRUCTION; tdi; "
              "instruction_select; active_signal; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(model.netlist.nodes.at(model.instruction_register).name !=
                  "INSTRUCTION" &&
              model.netlist.nodes.at(model.scan_input).name != "tdi",
          "internal names cannot collide with named ActiveSignals");
  for (const auto* name : {"long_setup", "INSTRUCTION", "tdi",
                           "instruction_select", "active_signal"}) {
    Require(
        ReadBit(model, {model.netlist.node_ids.at(name), 0}, "0011") == '1' &&
            ReadBit(model, {model.netlist.node_ids.at(name), 0}, "0100") == '0',
        "colliding names preserve user-visible ActiveSignal identity");
  }
  icl = Replace(Read(kDirectory + "simple.icl"),
                "ActiveSignals { long_setup; }", "");
  icl = Replace(icl, "InputPort mode = tap.long_setup;",
                "InputPort mode = 1'b0;");
  const auto no_signals = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(no_signals.instruction_selects.size() == 2 &&
              !no_signals.netlist.node_ids.count("long_setup"),
          "instructions without ActiveSignals still have decoders");
}
void TestManyAlternativesAndCaptureMask() {
  std::string codes = "0011";
  for (int i = 0; i < 2048; ++i) codes += ",0011";
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (0011)",
                      "IJTAG_ON (" + codes + ")");
  bsdl = Replace(bsdl, "is \"0001\";", "is \"XX01\";");
  const auto start = std::chrono::steady_clock::now();
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  const auto& capture = model.netlist.nodes.at(model.instruction_register)
                            .inputs.at("CaptureSource");
  std::string captured;
  for (const auto& bit : capture) captured += bit.constant;
  Require(captured == "xx01", "BSDL capture masks remain unknown, not zero");
  const auto select = model.instruction_selects.at("IJTAG_ON");
  Require(ReadBit(model, {select, 0}, "0011") == '1' &&
              ReadBit(model, {select, 0}, "0001") == '0',
          "balanced alternatives avoid expression-depth failures");
  const std::chrono::duration<double> elapsed =
      std::chrono::steady_clock::now() - start;
  std::cout << "TAP model: 2049 opcode alternatives, seconds="
            << elapsed.count() << '\n';
}
void TestMissingBinding() {
  pattern::AccessLink missing;
  missing.name = "tap";
  missing.origin = "missing.icl:1:1";
  try {
    pattern::BuildTapDatapathModel(missing);
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find("missing.icl:1:1: AccessLink tap:") == 0,
        "missing binding has source context");
    return;
  }
  throw std::runtime_error("missing BSDL binding accepted");
}
void TestIndexedSignals() {
  for (const auto* file :
       {"range_down.icl", "range_up.icl", "single_bit.icl"}) {
    pattern::Design design;
    design.AddIcl("base.icl", Read(kDirectory + "simple.icl"));
    design.AddIcl("probe.icl", Read(kDirectory + file));
    design.AddBsdl("probe.bsdl", Read(kDirectory + "simple.bsdl"),
                   "AccessProbe");
    const auto links =
        pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
    const auto model = pattern::BuildTapDatapathModel(links[0]);
    const auto id = model.netlist.node_ids.at("long_setup");
    const auto& node = model.netlist.nodes.at(id);
    const bool single = std::string(file) == "single_bit.icl";
    Require(node.indexed && node.Width() == (single ? 1 : 3),
            "indexed ActiveSignals retain their declared width");
    Require(node.left == (std::string(file) == "range_up.icl" ? 1 : 3) &&
                node.right == (single                                ? 3
                               : std::string(file) == "range_up.icl" ? 3
                                                                     : 1),
            "ActiveSignal range direction and nonzero index survive");
    for (int bit = single ? 3 : 1; bit <= 3; ++bit) {
      Require(ReadBit(model, {id, bit}, "0011") == '1' &&
                  ReadBit(model, {id, bit}, "0100") == '0' &&
                  ReadBit(model, {id, bit}, "xxxx") == 'x',
              "every selected bit follows instruction state");
    }
    const auto flat = pattern::SerializeIcl(model.netlist);
    pattern::Design replay;
    replay.AddIcl("flat.icl", flat);
    Require(pattern::SerializeIcl(
                pattern::Elaborate(replay, model.netlist.top_module)) == flat,
            "indexed instruction component round trip");
  }
}
void TestOverlappingParameterizedSignals() {
  auto icl = Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                     "Module AccessTop { Parameter N = 3; "
                     "LocalParameter M = $N - 1;");
  icl = Replace(icl, "ActiveSignals { long_setup; }",
                "ActiveSignals { long_setup; bus[$N:1]; bus[2]; "
                "sparse[5]; INSTRUCTION[$M:0]; }");
  icl = Replace(icl, "IJTAG_OFF {",
                "IJTAG_OFF { ActiveSignals { bus[2:4]; sparse[7]; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  const auto bus = model.netlist.node_ids.at("bus");
  const auto sparse = model.netlist.node_ids.at("sparse");
  Require(model.netlist.nodes.at(bus).left == 4 &&
              model.netlist.nodes.at(bus).right == 1,
          "overlapping selections extend the first range orientation");
  for (int bit = 1; bit <= 4; ++bit) {
    Require(ReadBit(model, {bus, bit}, "0011") == (bit <= 3 ? '1' : '0') &&
                ReadBit(model, {bus, bit}, "0100") == (bit >= 2 ? '1' : '0') &&
                ReadBit(model, {bus, bit}, "0000") == '0',
            "overlapping ranges OR selection per bit, not per whole port");
  }
  Require(ReadBit(model, {sparse, 5}, "0011") == '1' &&
              ReadBit(model, {sparse, 7}, "0011") == '0' &&
              ReadBit(model, {sparse, 6}, "xxxx") == '0',
          "unlisted sparse bits are inactive even with unknown IR");
  Require(model.netlist.NodeAt("INSTRUCTION").Width() == 3 &&
              model.netlist.nodes.at(model.instruction_register).name !=
                  "INSTRUCTION",
          "parameterized ActiveSignal reserves its base name");
}
void TestExtremeSignalIndex() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; high[9223372036854775807]; }");
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  Require(ReadBit(model,
                  {model.netlist.node_ids.at("high"),
                   std::numeric_limits<int64_t>::max()},
                  "0011") == '1',
          "single bit at maximum index does not overflow traversal");
}
void TestInvalidSignalRanges() {
  for (const auto& probe : std::vector<std::pair<std::string, std::string>>{
           {"bus; bus[0];", "scalar/indexed ActiveSignal conflict"},
           {"bus[0]; bus;", "scalar/indexed ActiveSignal conflict"},
           {"bus[0]; bus[9223372036854775807];", "width limit"},
           {"bus[$missing];", "unknown parameter"},
           {"bus[2-3];", "negative ICL integer"},
           {"bus[1048576:0];", "width limit"}}) {
    const auto icl = Replace(
        Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
        "ActiveSignals { long_setup; " + probe.first + " }");
    bool rejected = false;
    try {
      Build(icl, Read(kDirectory + "simple.bsdl"));
    } catch (const std::runtime_error& error) {
      const std::string message = error.what();
      rejected = message.find("probe.icl:") != std::string::npos &&
                 message.find(probe.second) != std::string::npos;
      Require(rejected, "unexpected range diagnostic: " + message);
    }
    Require(rejected, "invalid or excessive ActiveSignal range accepted");
  }
}
void TestSignalDiagnosticOrigin() {
  pattern::Design design;
  design.AddIcl(
      "range_error.icl",
      Replace(
          Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                  "Module AccessTop { Parameter IclPatternSignalShapes = 3;"),
          "ActiveSignals { long_setup; }",
          "ActiveSignals { long_setup;\n bus[$missing]; }"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  const auto& bad = links[0].instructions[0].active_signals.back();
  try {
    pattern::BuildTapDatapathModel(links[0]);
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find(bad.origin + ":") != std::string::npos,
        "range errors identify original ActiveSignal location: " +
            std::string(error.what()));
    return;
  }
  throw std::runtime_error("undefined ActiveSignal parameter accepted");
}
void TestOverlappingRangeBudget() {
  std::string signals;
  for (int i = 0; i < 18; ++i) {
    signals += "bus[65535:" + std::to_string(i) + "];";
  }
  const auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; " + signals + " }");
  try {
    Build(icl, Read(kDirectory + "simple.bsdl"));
  } catch (const std::runtime_error& error) {
    Require(
        std::string(error.what()).find("selection limit") != std::string::npos,
        "bounded expansion of distinct overlapping ranges");
    return;
  }
  throw std::runtime_error("overlapping ranges evade work budget");
}
void TestDuplicateRangeScale() {
  std::string signals;
  for (int i = 0; i < 10000; ++i) signals += "bus[65535:0];";
  const auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; " + signals + " }");
  const auto start = std::chrono::steady_clock::now();
  const auto model = Build(icl, Read(kDirectory + "simple.bsdl"));
  const auto bus = model.netlist.node_ids.at("bus");
  Require(ReadBit(model, {bus, 0}, "0011") == '1' &&
              ReadBit(model, {bus, 65535}, "0100") == '0',
          "duplicate ranges retain bit semantics without duplicate work");
  std::cout << "TAP model: 10000 repeated 65536-bit ranges, seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestCopiedTopLifetime() {
  pattern::Design design;
  design.AddIcl("copy.icl",
                Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                        "Module AccessTop { Parameter N = 2;"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  std::vector<pattern::AccessLink> links;
  {
    const auto top = design.SelectTop("AccessTop");
    links = pattern::BindAccessLinks(design, top);
  }
  const auto model = pattern::BuildTapDatapathModel(links[0]);
  Require(ReadBit(model, {model.netlist.node_ids.at("long_setup"), 0},
                  "0011") == '1',
          "binding lifetime depends on Design, not a temporary top copy");
}
pattern::BitReference MuxSource(const pattern::NetlistNode& mux,
                                const std::string& selector) {
  std::optional<pattern::BitReference> result;
  for (const auto& choice : mux.choices) {
    for (const auto& code : choice.codes) {
      Require(code.data.size() == selector.size(), "selector width");
      bool match = true;
      for (size_t bit = 0; bit < selector.size(); ++bit) {
        if (code.data[bit] != 'x' && code.data[bit] != selector[bit])
          match = false;
      }
      if (match) {
        Require(!result || (result->node == choice.source.at(0).node &&
                            result->index == choice.source.at(0).index),
                "one register source per instruction");
        result = choice.source.at(0);
      }
    }
  }
  Require(result.has_value(), "instruction has a scan path");
  return *result;
}
void TestDataRegisterRoutes() {
  const auto model =
      Build(Read(kDirectory + "simple.icl"), Read(kDirectory + "simple.bsdl"));
  const auto& nodes = model.netlist.nodes;
  const auto ir_mux_id =
      nodes.at(model.scan_output).inputs.at("Source")[0].node;
  const auto& ir_mux = nodes.at(ir_mux_id);
  Require(ir_mux.inputs.at("SelectedBy")[0].node == model.ir_select &&
              nodes.at(model.ir_select).kind == "DataInPort" &&
              MuxSource(ir_mux, "1").node == model.instruction_register,
          "IR select chooses instruction register scan output");
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  const auto& dr_mux = nodes.at(MuxSource(fallback, "1").node);
  for (const auto& probe :
       std::vector<std::pair<std::string, std::string>>{{"0000", "BOUNDARY"},
                                                        {"0001", "BOUNDARY"},
                                                        {"0010", "DEVICE_ID"},
                                                        {"1111", "BYPASS"}}) {
    const auto source = MuxSource(dr_mux, probe.first);
    Require(source.node == model.data_registers.at(probe.second) &&
                source.index == 0,
            "documented opcode selects the expected BSDL register");
  }
  const auto& bypass = nodes.at(model.data_registers.at("BYPASS"));
  Require(bypass.Width() == 1 &&
              bypass.inputs.at("CaptureSource")[0].constant == '0',
          "bypass captures zero");
  const auto& idcode = nodes.at(model.data_registers.at("DEVICE_ID"));
  std::string captured;
  for (const auto& bit : idcode.inputs.at("CaptureSource"))
    captured += bit.constant;
  Require(
      idcode.Width() == 32 && captured == "00010000000000000000000000000011",
      "IDCODE captures the BSDL value");
  Require(nodes.at(model.data_registers.at("BOUNDARY")).Width() == 1 &&
              !nodes.at(model.data_registers.at("BOUNDARY"))
                   .inputs.count("CaptureSource"),
          "boundary state is not invented from a scan length");
  for (const auto& probe : std::vector<std::pair<std::string, std::string>>{
           {"0011", "IJTAG_ON"}, {"0100", "IJTAG_OFF"}}) {
    const auto returned = model.scan_returns.at(probe.second);
    Require(MuxSource(dr_mux, probe.first).node == returned &&
                nodes.at(returned).kind == "ScanInPort",
            "selected AccessLink chain has its own scan-return boundary");
  }
  for (const auto& entry : model.data_registers) {
    const auto& reg = nodes.at(entry.second);
    Require(reg.inputs.at("ScanInSource")[0].node == model.scan_input &&
                !reg.reset && !reg.constants.count("ResetValue"),
            "data paths share TDI without silently assuming reset");
  }
}
void TestImplicitStandardRegisters() {
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"),
                      "  attribute REGISTER_ACCESS of AccessProbe : entity is\n"
                      "    \"BOUNDARY (EXTEST, SAMPLE), BYPASS (BYPASS), "
                      "DEVICE_ID (IDCODE)\";\n",
                      "");
  bsdl = Replace(bsdl, "SAMPLE (0001)",
                 "SAMPLE (0001), PRELOAD (0101), INTEST (0110), "
                 "CLAMP (0111), HIGHZ (1000)");
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  const auto& nodes = model.netlist.nodes;
  const auto& ir_mux =
      nodes.at(nodes.at(model.scan_output).inputs.at("Source")[0].node);
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  const auto& dr_mux = nodes.at(MuxSource(fallback, "1").node);
  for (const auto& probe :
       std::vector<std::pair<std::string, std::string>>{{"0000", "BOUNDARY"},
                                                        {"0001", "BOUNDARY"},
                                                        {"0101", "BOUNDARY"},
                                                        {"0110", "BOUNDARY"},
                                                        {"0010", "DEVICE_ID"},
                                                        {"0111", "BYPASS"},
                                                        {"1000", "BYPASS"},
                                                        {"1111", "BYPASS"}}) {
    Require(MuxSource(dr_mux, probe.first).node ==
                model.data_registers.at(probe.second),
            "standard instruction has its mandated implicit register");
  }
  const auto flat = pattern::SerializeIcl(model.netlist);
  pattern::Design replay;
  replay.AddIcl("implicit-flat.icl", flat);
  Require(pattern::SerializeIcl(
              pattern::Elaborate(replay, model.netlist.top_module)) == flat,
          "implicit register mappings survive flat replay");
}
void TestUnassignedOpcodes() {
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IJTAG_ON (0011)",
                      "IJTAG_ON (0011, 10XX)");
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  const auto& nodes = model.netlist.nodes;
  const auto& ir_mux =
      nodes.at(nodes.at(model.scan_output).inputs.at("Source")[0].node);
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  Require(fallback.inputs.at("SelectedBy").size() == 1,
          "DR selection distinguishes assigned and unassigned opcodes");
  const auto bypass = MuxSource(fallback, "0");
  Require(bypass.node == model.data_registers.at("BYPASS"),
          "unassigned binary opcodes select the shared BYPASS register");
  const auto& assigned = nodes.at(MuxSource(fallback, "1").node);
  for (int value = 0; value < 16; ++value) {
    std::string code(4, '0');
    for (int bit = 0; bit < 4; ++bit) code[3 - bit] += (value >> bit) & 1;
    const char known =
        ReadBit(model, fallback.inputs.at("SelectedBy")[0], code);
    Require(known == ((value <= 4 || (value >= 8 && value <= 11) || value == 15)
                          ? '1'
                          : '0'),
            "literal and masked instruction coverage is exact");
    if (known == '1') MuxSource(assigned, code);
  }
  Require(ReadBit(model, fallback.inputs.at("SelectedBy")[0], "xxxx") == 'x',
          "unknown instruction bits do not imply a known BYPASS state");
}
void TestUsercodeCapture() {
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "IDCODE (0010)",
                      "IDCODE (0010), USERCODE (0101), CUSTOM_ID (0110)");
  bsdl = Replace(bsdl, "DEVICE_ID (IDCODE)", "DEVICE_ID (IDCODE, CUSTOM_ID)");
  bsdl = Replace(bsdl, "end AccessProbe;",
                 "attribute USERCODE_REGISTER of AccessProbe : entity is\n"
                 "\"10101010101010101010101010101010\";\nend AccessProbe;");
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  const auto& nodes = model.netlist.nodes;
  const auto& capture =
      nodes.at(model.data_registers.at("DEVICE_ID")).inputs.at("CaptureSource");
  Require(capture.size() == 32 &&
              capture[0].node != std::numeric_limits<size_t>::max(),
          "USERCODE must not inherit constant IDCODE capture data");
  const auto& mux = nodes.at(capture[0].node);
  Require(mux.kind == "DataMux",
          "identification capture depends on instruction");
  for (const auto& probe : std::vector<std::pair<std::string, std::string>>{
           {"0010", "00010000000000000000000000000011"},
           {"0101", "10101010101010101010101010101010"},
           {"0110", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}}) {
    std::string value;
    for (const auto& choice : mux.choices) {
      for (const auto& code : choice.codes) {
        if (code.data == probe.first)
          for (const auto& bit : choice.source) value += bit.constant;
      }
    }
    Require(value == probe.second,
            "instruction-specific identification capture");
  }
  bool rejected = false;
  try {
    Build(Read(kDirectory + "simple.icl"),
          Replace(bsdl, "USERCODE_REGISTER", "UNKNOWN_REGISTER"));
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("USERCODE_REGISTER") !=
               std::string::npos;
  }
  Require(rejected, "missing usercode value must not silently use IDCODE");
  rejected = false;
  const auto aliases = Replace(bsdl, "USERCODE (0101)", "USERCODE (0010)");
  try {
    Build(Read(kDirectory + "simple.icl"), aliases);
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("conflicting identification") !=
               std::string::npos;
  }
  Require(rejected, "same register route cannot hide conflicting captures");
  Build(Read(kDirectory + "simple.icl"),
        Replace(aliases, "10101010101010101010101010101010",
                "00010000000000000000000000000011"));
  const auto flat = pattern::SerializeIcl(model.netlist);
  pattern::Design replay;
  replay.AddIcl("capture-flat.icl", flat);
  Require(pattern::SerializeIcl(
              pattern::Elaborate(replay, model.netlist.top_module)) == flat,
          "instruction-dependent capture survives flat replay");
}
void TestInvalidStandardAssociations() {
  const auto original = Read(kDirectory + "simple.bsdl");
  for (const auto& changed :
       {Replace(original, "BYPASS (BYPASS)", "BOUNDARY (BYPASS)"),
        Replace(original, "DEVICE_ID (IDCODE)", "OTHER[32] (IDCODE)"),
        Replace(original, "BOUNDARY (EXTEST, SAMPLE)",
                "BYPASS (EXTEST, SAMPLE)")}) {
    bool rejected = false;
    try {
      Build(Read(kDirectory + "simple.icl"), changed);
    } catch (const std::runtime_error& error) {
      const std::string message = error.what();
      rejected = message.find("mandated register") != std::string::npos &&
                 message.find("probe.bsdl:") != std::string::npos;
    }
    Require(rejected, "contradictory standard association must be rejected");
  }
  bool rejected = false;
  try {
    Build(Replace(Read(kDirectory + "simple.icl"), "IJTAG_ON {", "EXTEST {"),
          Replace(original, "BOUNDARY (EXTEST, SAMPLE)",
                  "BOUNDARY (SAMPLE), BYPASS (EXTEST)"));
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("mandated register") !=
               std::string::npos;
  }
  Require(rejected, "AccessLink route must not hide invalid BSDL association");
  rejected = false;
  try {
    Build(Replace(Read(kDirectory + "simple.icl"), "IJTAG_ON {", "IDCODE {"),
          Replace(original, "DEVICE_ID (IDCODE)", "DEVICE_ID[16] (IDCODE)"));
  } catch (const std::runtime_error& error) {
    rejected = std::string(error.what()).find("inconsistent width") !=
               std::string::npos;
  }
  Require(rejected, "AccessLink route must not hide invalid standard width");
}
void TestInvalidDataRoutes() {
  auto icl = Replace(Read(kDirectory + "simple.icl"), "Module AccessTop {",
                     "Module AccessTop { Instance other Of AccessInstrument;");
  icl = Replace(icl, "IJTAG_OFF { ScanInterface { core.client; } }",
                "IJTAG_OFF { ScanInterface { other.client; } }");
  const auto bsdl = Read(kDirectory + "simple.bsdl");
  for (const auto& probe : std::vector<std::pair<std::string, std::string>>{
           {Replace(bsdl, "IJTAG_OFF (0100)", "IJTAG_OFF (0011)"),
            "conflicting"},
           {Replace(bsdl, "IJTAG_ON (0011)", "IJTAG_ON (0XXX)"), "conflicting"},
           {Replace(bsdl, "BYPASS (1111)", "BYPASS (XXXX)"), "conflicting"},
           {Replace(bsdl, "BYPASS (BYPASS)", "UNKNOWN (BYPASS)"),
            "mandated register"},
           {Replace(UserRegisterBsdl(), "USER[9]", "USER[1048576]"),
            "width limit"},
           {Replace(UserRegisterBsdl(), "USER[9]", "USER"), "width"},
           {Replace(bsdl, "DEVICE_ID (IDCODE)", "DEVICE_ID[16] (IDCODE)"),
            "width"},
           {Replace(bsdl, "IDCODE_REGISTER", "MISSING_IDCODE_REGISTER"),
            "missing IDCODE_REGISTER"},
           {Replace(Replace(bsdl, "BYPASS (1111)", "CUSTOM (1111)"),
                    "BYPASS (BYPASS)", "USER[1] (CUSTOM)"),
            "missing BYPASS"},
           {Replace(bsdl, "BYPASS (BYPASS)", "BYPASS[2] (BYPASS)"), "width"},
           {Replace(bsdl, "SAMPLE (0001)", "SAMPLE (0001), UNMAPPED (1000)"),
            "association"},
           {Replace(UserRegisterBsdl(), "USER[9] (USER_ONE, USER_TWO)",
                    "USER[2] (USER_ONE), USER[3] (USER_TWO)"),
            "inconsistent"}}) {
    bool rejected = false;
    try {
      Build(icl, probe.first);
    } catch (const std::runtime_error& error) {
      const std::string message = error.what();
      rejected = message.find(probe.second) != std::string::npos &&
                 message.find("probe.bsdl:") != std::string::npos &&
                 message.find("AccessLink tap") != std::string::npos;
      Require(rejected, "unexpected data-route diagnostic: " + message);
    }
    Require(rejected, "invalid BSDL data-route mapping accepted");
  }
}
void TestSharedRegisterRoutesAndNames() {
  auto icl =
      Replace(Read(kDirectory + "simple.icl"), "ActiveSignals { long_setup; }",
              "ActiveSignals { long_setup; BYPASS; BOUNDARY; DEVICE_ID; "
              "ir_select; tdo; scan_return; data_mux; ir_mux; }");
  const auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "SAMPLE (0001)",
                            "SAMPLE (0000, 0001)");
  const auto model = Build(icl, bsdl);
  const auto& nodes = model.netlist.nodes;
  for (const auto& entry : model.data_registers) {
    Require(nodes.at(entry.second).name != entry.first,
            "register identities cannot collide with user ActiveSignals");
  }
  Require(nodes.at(model.ir_select).name != "ir_select" &&
              nodes.at(model.scan_output).name != "tdo",
          "datapath boundary names avoid ActiveSignals");
  const auto& ir_mux =
      nodes.at(nodes.at(model.scan_output).inputs.at("Source")[0].node);
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  const auto& dr_mux = nodes.at(MuxSource(fallback, "1").node);
  Require(MuxSource(dr_mux, "0000").node == model.data_registers.at("BOUNDARY"),
          "multiple instruction aliases can select the same register");
  const auto custom =
      Build(Read(kDirectory + "simple.icl"), UserRegisterBsdl());
  Require(
      custom.netlist.nodes.at(custom.data_registers.at("USER")).Width() == 9 &&
          !custom.netlist.nodes.at(custom.data_registers.at("USER"))
               .inputs.count("CaptureSource"),
      "explicit-width user registers preserve unknown capture state");
}
void TestSharedInterfaceRoutes() {
  const auto bsdl = Replace(Read(kDirectory + "simple.bsdl"),
                            "IJTAG_OFF (0100)", "IJTAG_OFF (0011)");
  const auto model = Build(Read(kDirectory + "simple.icl"), bsdl);
  Require(
      model.scan_returns.at("IJTAG_ON") == model.scan_returns.at("IJTAG_OFF"),
      "opcode aliases of the same ordered chain share one return boundary");
  Require(ReadBit(model, {model.netlist.node_ids.at("long_setup"), 0},
                  "0011") == '1',
          "alias activation is the union of matching instructions");
}
std::string WideBsdl(const std::string& bypass_codes) {
  auto bsdl = Replace(Read(kDirectory + "simple.bsdl"), "is 4;", "is 16;");
  bsdl = Replace(bsdl, "is \"0001\";", "is \"0000000000000001\";");
  for (const auto& code : std::vector<std::pair<std::string, std::string>>{
           {"EXTEST (0000)", "EXTEST (0000000000000000)"},
           {"SAMPLE (0001)", "SAMPLE (0000000000000001)"},
           {"IDCODE (0010)", "IDCODE (0000000000000010)"},
           {"IJTAG_ON (0011)", "IJTAG_ON (0000000000000011)"},
           {"IJTAG_OFF (0100)", "IJTAG_OFF (0000000000000100)"}}) {
    bsdl = Replace(bsdl, code.first, code.second);
  }
  return Replace(bsdl, "BYPASS (1111)",
                 "BYPASS (1111111111111111," + bypass_codes + ")");
}
std::string Binary16(int value) {
  std::string result(16, '0');
  for (int bit = 0; bit < 16; ++bit) result[15 - bit] += (value >> bit) & 1;
  return result;
}
void TestMaskedRouteBudget() {
  std::string codes;
  for (int i = 0; i < 6000; ++i) {
    auto code = Binary16(32768 + i * 2);
    code.back() = 'X';
    if (!codes.empty()) codes += ",";
    codes += code;
  }
  try {
    Build(Read(kDirectory + "simple.icl"), WideBsdl(codes));
  } catch (const std::runtime_error& error) {
    Require(std::string(error.what()).find("opcode overlap work limit") !=
                std::string::npos,
            "distinct masked opcode comparison is bounded");
    return;
  }
  throw std::runtime_error("masked alternatives bypass overlap work limit");
}
void TestManyExactRoutes() {
  std::string codes;
  for (int i = 256; i < 10256; ++i) {
    if (!codes.empty()) codes += ",";
    codes += Binary16(i);
  }
  const auto start = std::chrono::steady_clock::now();
  const auto model = Build(Read(kDirectory + "simple.icl"), WideBsdl(codes));
  const auto& nodes = model.netlist.nodes;
  const auto& ir_mux =
      nodes.at(nodes.at(model.scan_output).inputs.at("Source")[0].node);
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  const auto& dr_mux = nodes.at(MuxSource(fallback, "1").node);
  Require(MuxSource(dr_mux, "0000000100000000").node ==
                  model.data_registers.at("BYPASS") &&
              MuxSource(dr_mux, "0010100000001111").node ==
                  model.data_registers.at("BYPASS"),
          "10000 unique literal opcode routes avoid quadratic mask checks");
  std::cout << "TAP model: 10000 additional literal opcodes, seconds="
            << std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
                   .count()
            << '\n';
}
void TestActiveOnlyInstructions() {
  auto icl = Replace(Read(kDirectory + "simple.icl"),
                     "IJTAG_OFF { ScanInterface { core.client; } }",
                     "IJTAG_OFF { ActiveSignals { off; } }");
  icl = Replace(icl, "ScanInterface { core.client; }", "");
  const auto bsdl =
      Replace(Read(kDirectory + "simple.bsdl"), "BOUNDARY (EXTEST, SAMPLE)",
              "BOUNDARY (EXTEST, SAMPLE), USER[8] (IJTAG_ON, IJTAG_OFF)");
  const auto model = Build(icl, bsdl);
  Require(
      model.scan_returns.empty() &&
          model.netlist.nodes.at(model.data_registers.at("USER")).Width() == 8,
      "ActiveSignal-only instructions use their declared data register");
  const auto& nodes = model.netlist.nodes;
  const auto& ir_mux =
      nodes.at(nodes.at(model.scan_output).inputs.at("Source")[0].node);
  const auto& fallback = nodes.at(MuxSource(ir_mux, "0").node);
  const auto& dr_mux = nodes.at(MuxSource(fallback, "1").node);
  Require(MuxSource(dr_mux, "0011").node == model.data_registers.at("USER") &&
              MuxSource(dr_mux, "0100").node == model.data_registers.at("USER"),
          "ActiveSignal-only alias instructions share declared register path");
}
void TestInstructionModel() {
  pattern::Design design;
  design.AddIcl("simple.icl", Read(kDirectory + "simple.icl"));
  design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                 "AccessProbe");
  const auto links =
      pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
  const auto model = pattern::BuildTapDatapathModel(links[0]);
  const auto& netlist = model.netlist;
  const auto& reg = netlist.nodes.at(model.instruction_register);
  Require(reg.kind == "ScanRegister" && reg.Width() == 4 &&
              reg.inputs.at("ScanInSource")[0].node == model.scan_input &&
              reg.inputs.at("CaptureSource").size() == 4 &&
              reg.inputs.at("CaptureSource")[3].constant == '1' &&
              reg.inputs.at("CaptureSource")[0].constant == '0',
          "instruction width/capture/scan input reuse elaborated ICL nodes");
  Require(!reg.reset && !reg.constants.count("ResetValue"),
          "component construction does not imply reset or initialized state");
  const auto source = netlist.NodeAt("long_setup").inputs.at("Source")[0];
  for (int bits = 0; bits < 16; ++bits) {
    const auto value = [&](const pattern::BitReference& bit) {
      Require(bit.node == model.instruction_register,
              "decoder reads instruction register");
      return ((bits >> bit.index) & 1) ? '1' : '0';
    };
    const auto& on = netlist.nodes.at(model.instruction_selects.at("IJTAG_ON"));
    const auto& off =
        netlist.nodes.at(model.instruction_selects.at("IJTAG_OFF"));
    Require(pattern::EvaluateLogic(on, value) == (bits == 3 ? '1' : '0') &&
                pattern::EvaluateLogic(off, value) == (bits == 4 ? '1' : '0'),
            "all instruction states agree with oracle opcodes");
    Require(source.node == model.instruction_selects.at("IJTAG_ON"),
            "ActiveSignal uses shared instruction decoder");
  }
  Require(pattern::EvaluateLogic(
              netlist.nodes.at(source.node),
              [](const pattern::BitReference&) { return 'x'; }) == 'x',
          "uninitialized instruction is unknown");
  const auto flat = pattern::SerializeIcl(netlist);
  pattern::Design replay;
  replay.AddIcl("flat.icl", flat);
  const auto again = pattern::Elaborate(replay, netlist.top_module);
  Require(pattern::SerializeIcl(again) == flat,
          "instruction component flat replay");
}
void TestDatapathInsertion() {
  pattern::Netlist netlist;
  std::string source;
  {
    pattern::Design design;
    source = Read(kDirectory + "simple.icl");
    design.AddIcl("original.icl", source);
    design.AddBsdl("simple.bsdl", Read(kDirectory + "simple.bsdl"),
                   "AccessProbe");
    const auto links =
        pattern::BindAccessLinks(design, design.SelectTop("AccessTop"));
    const auto model = pattern::BuildTapDatapathModel(links.front());
    netlist = pattern::Elaborate(design, "AccessTop",
                                 {{"tap", model.netlist.scopes[0].definition}});
    Require(netlist.scopes[0].definition.document ==
                design.modules().at("AccessTop").document,
            "integrated datapath retains original shared source tree");
    const auto& original = model.netlist.nodes[model.instruction_register];
    const auto& integrated = netlist.NodeAt("tap." + original.name);
    Require(integrated.ast == original.ast && integrated.Width() == 4 &&
                integrated.inputs.at("CaptureSource")[3].constant == '1',
            "the behavioral instruction register reuses its parsed model AST");
  }
  Require(netlist.scopes[0].definition.document->source() == source &&
              netlist.NodeAt("core.mode").inputs.at("Source")[0].node ==
                  netlist.node_ids.at("tap.long_setup"),
          "qualified active signal binding survives source/model owner "
          "destruction");
  const auto flat = pattern::SerializeIcl(netlist);
  pattern::Design replay;
  replay.AddIcl("flat.icl", flat);
  const auto restored = pattern::Elaborate(replay, "AccessTop");
  Require(
      pattern::SerializeIcl(restored) == flat &&
          restored.NodeAt("core.mode").inputs.at("Source")[0].node ==
              restored.node_ids.at("tap.long_setup"),
      "inserted datapath preserves qualified connections through flat replay");
}

}  // namespace
int main() {
  try {
    TestDatapathInsertion();
    TestControllerTransitions();
    TestControllerEdgesAndReset();
    TestInstructionExecution();
    TestInstructionResetAlternatives();
    TestInstructionWithoutTrst();
    TestInstructionUnknownsAndValidation();
    TestInstructionShiftScale();
    TestInstructionUncertainUpdate();
    TestInstructionStateLimits();
    TestInstructionDecoderIntegration();
    TestUnassignedOpcodes();
    TestUsercodeCapture();
    TestImplicitStandardRegisters();
    TestInvalidStandardAssociations();
    TestActiveOnlyInstructions();
    TestMaskedRouteBudget();
    TestManyExactRoutes();
    TestSharedInterfaceRoutes();
    TestInvalidDataRoutes();
    TestSharedRegisterRoutesAndNames();
    TestDataRegisterRoutes();
    TestCopiedTopLifetime();
    TestSignalDiagnosticOrigin();
    TestOverlappingRangeBudget();
    TestDuplicateRangeScale();
    TestExtremeSignalIndex();
    TestInvalidSignalRanges();
    TestInstructionModel();
    TestSharedSignalsAndMasks();
    TestNamesAndMissingSignals();
    TestIndexedSignals();
    TestOverlappingParameterizedSignals();
    TestManyAlternativesAndCaptureMask();
    TestMissingBinding();
    std::cout << "pattern TAP datapath tests passed\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
