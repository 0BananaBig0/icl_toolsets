// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_NETLIST_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_NETLIST_H_

#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "pattern/pattern_design.h"
#include "pattern/pattern_expression.h"

namespace pattern {
constexpr size_t kMaxAddressBits = 4 * 1024 * 1024;

struct Expression {
  size_t scope = 0;
  size_t begin = 0, end = 0;
};

enum class VirtualSignal { kNone, kGlobalReset, kAsyncGlobalReset };
enum class ModelKind {
  kPort,
  kRegister,
  kMux,
  kLogic,
  kAlias,
  kEnum,
  kInterface,
  kGroup,
  kAccessLink
};

struct BitReference {
  // A missing node denotes a literal bit rather than an object connection.
  size_t node = std::numeric_limits<size_t>::max();
  int64_t index = 0;
  char constant = 'x';
  bool inverted = false;
  VirtualSignal virtual_signal = VirtualSignal::kNone;
};

struct MuxChoice {
  std::vector<Constant> codes;
  std::vector<BitReference> source;
};

enum class OneHotSourceKind { kPort, kInstance, kRegister };

struct OneHotSource {
  std::vector<BitReference> bits;
  OneHotSourceKind kind = OneHotSourceKind::kPort;
  // Instance scope ID or DataRegister node ID; unused for Port contributors.
  size_t object = std::numeric_limits<size_t>::max();
};

enum class LogicOperation {
  kLiteral,
  kSignal,
  kConcat,
  kInvert,
  kNot,
  kAnd,
  kOr,
  kXor,
  kReduceAnd,
  kReduceOr,
  kReduceXor,
  kEqual,
  kNotEqual
};

struct LogicInstruction {
  LogicOperation operation = LogicOperation::kLiteral;
  size_t left = 0, right = 0, width = 0;
  Constant literal;
  std::vector<BitReference> signals;
};

using Attributes = std::map<std::string, std::optional<Constant>>;

enum class ScanInterfaceKind { kClient, kHost, kClientTap, kHostTap };

struct ScanChain {
  std::string name;
  std::vector<BitReference> ports;
  std::optional<Constant> default_load;
  Attributes attributes;
};

struct ScanInterface {
  ScanInterfaceKind kind = ScanInterfaceKind::kClient;
  std::vector<BitReference> ports;
  std::optional<Constant> default_load;
  std::vector<ScanChain> chains;
};

struct NetlistNode {
  std::string name, kind, origin;
  ModelKind model = ModelKind::kPort;
  size_t scope = 0;
  const icl::AstNode* ast = nullptr;
  int64_t left = 0, right = 0;
  bool indexed = false;
  bool active_low = false;
  std::optional<size_t> enumeration;
  std::map<std::string, Expression> properties;
  std::map<std::string, std::vector<BitReference>> inputs;
  std::map<std::string, Constant> constants;
  std::optional<BitReference> reset;
  std::vector<MuxChoice> choices;
  // Original contributor widths; narrower data sources are right-aligned with X
  // in the leading output positions. Keep raw widths for lossless ICL replay.
  std::vector<OneHotSource> one_hot_sources;
  std::vector<LogicInstruction> logic;
  Attributes attributes;
  std::optional<ScanInterface> scan_interface;
  size_t Width() const {
    return static_cast<size_t>(left > right ? left - right : right - left) + 1;
  }
  // position is an offset in group declaration order, not a numeric bit index.
  BitReference OneHotSourceBit(size_t source, size_t position) const;
};

struct InstanceScope {
  std::string path;
  ModuleDefinition definition;
  size_t parent = std::numeric_limits<size_t>::max();
  std::map<std::string, Constant> parameters;
  std::map<std::string, size_t> members;
  std::vector<size_t> children;
  Attributes attributes;
  Attributes instance_attributes;
  std::optional<Constant> address_value;
};

struct TapDatapathNodes {
  size_t instruction_register = 0;
  size_t scan_input = 0;
  size_t scan_output = 0;
  size_t ir_select = 0;
  std::map<std::string, size_t> instruction_selects;
  std::map<std::string, size_t> data_registers;
  std::map<std::string, size_t> scan_returns;
};

struct TapRuntimeModel : TapDatapathNodes {
  BsdlTap bsdl;
  std::map<std::string, size_t> pins;
  std::map<std::string, size_t> controls;
};

struct Netlist {
  std::string top_module;
  std::map<std::string, std::set<std::string>> module_parameters;
  std::vector<InstanceScope> scopes;
  std::vector<NetlistNode> nodes;
  std::map<std::string, size_t> node_ids;
  std::map<std::string, TapRuntimeModel> taps;
  const NetlistNode& NodeAt(const std::string& path) const;
  BitReference OneHotPortEnable(const OneHotSource& source) const;
  int64_t Integer(const Expression& expression) const;
  Constant Value(const Expression& expression, size_t width = 0) const;
  Expression ExpressionAt(size_t scope, icl::SourceSpan span) const;
  std::vector<BitReference> Resolve(const Expression& expression,
                                    size_t width = 0, bool scan_input = false,
                                    size_t max_bits = 1024 * 1024) const;
};

bool IsInputPort(const std::string& kind);
struct ScanInterfaceBinding {
  size_t scope;
  ScanInterface interface;
  std::string origin;
};
// Includes omitted single-client declarations (IEEE 6.4.6.11/6.4.16).
std::vector<ScanInterfaceBinding> EffectiveScanInterfaces(
    const Netlist& netlist);
Netlist Elaborate(Design& design, const std::string& top);
// Substitute parsed leaf behavioral modules for top-level standard AccessLinks
// before resolving connections. Does not rewrite source/module definitions or
// wire TAP defaults. Ordinary top-selection warnings still apply.
// Model identities must be distinct from other definitions unless sharing the
// same AST. Retained ModuleDefinitions keep generated ASTs alive in the result.
// top_ports maps valid ICL identifiers to scalar TAP port kinds; matching
// existing declarations are reused, and conflicting kinds/widths are rejected.
Netlist Elaborate(
    Design& design, const std::string& top,
    const std::map<std::string, ModuleDefinition>& access_link_models,
    const std::map<std::string, std::string>& top_ports = {});
// Recheck bit-level cycles/group fan-in after adding typed connections.
void CheckCombinationalConnections(const Netlist& netlist);
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_NETLIST_H_
