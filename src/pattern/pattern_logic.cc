// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern/pattern_logic.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace pattern {
namespace {
class Compiler {
 public:
  Compiler(Netlist* netlist, size_t node, size_t* remaining_bits)
      : netlist_(*netlist),
        node_(netlist->nodes.at(node)),
        remaining_bits_(*remaining_bits) {}

  void Run() {
    if (node_.Width() != 1) {
      throw std::runtime_error("LogicSignal must be one bit");
    }
    const auto* block = dynamic_cast<const icl::AstBlock*>(node_.ast);
    for (const auto* item : block->body()) {
      const auto* statement = dynamic_cast<const icl::AstStatement*>(item);
      if (!statement || !statement->Logic()) continue;
      const auto root = Build(*statement->Logic(), 0);
      Size(root, 1);
      auto& dependencies = node_.inputs["LogicDependencies"];
      std::vector<std::pair<size_t, int64_t>> referenced;
      for (const auto& instruction : node_.logic) {
        for (const auto& bit : instruction.signals) {
          if (bit.node != std::numeric_limits<size_t>::max()) {
            referenced.emplace_back(bit.node, bit.index);
          }
        }
      }
      std::sort(referenced.begin(), referenced.end());
      referenced.erase(std::unique(referenced.begin(), referenced.end()),
                       referenced.end());
      for (const auto& bit : referenced) {
        dependencies.push_back({bit.first, bit.second});
      }
      return;
    }
    throw std::runtime_error(
        "LogicSignal is missing shared expression metadata");
  }

 private:
  void Consume(size_t bits) {
    if (bits > remaining_bits_) {
      throw std::runtime_error("LogicSignal operand bit limit exceeded");
    }
    remaining_bits_ -= bits;
  }

  void Append(LogicInstruction term) {
    Consume(std::max(term.width, term.literal.data.size()));
    node_.logic.push_back(std::move(term));
  }

  void Size(size_t index, size_t width) {
    if (!width || width > 1024 * 1024) {
      throw std::runtime_error("ambiguous or excessive logic operand width");
    }
    auto& term = node_.logic.at(index);
    if (term.width) {
      if (term.width != width) {
        throw std::runtime_error("logic operand width mismatch");
      }
      return;
    }
    if (width > term.literal.data.size()) {
      Consume(width - term.literal.data.size());
    }
    switch (term.operation) {
      case LogicOperation::kLiteral:
        term.literal = term.literal.Sized(width);
        break;
      case LogicOperation::kConcat: {
        const auto left_width = node_.logic[term.left].width;
        const auto right_width = node_.logic[term.right].width;
        if ((!left_width && !right_width) ||
            left_width + right_width >= width) {
          throw std::runtime_error("cannot infer unsized logic concatenation");
        }
        Size(term.left, left_width ? left_width : width - right_width);
        Size(term.right, right_width ? right_width : width - left_width);
        break;
      }
      case LogicOperation::kInvert:
        Size(term.left, width);
        break;
      case LogicOperation::kAnd:
      case LogicOperation::kOr:
      case LogicOperation::kXor:
        Size(term.left, width);
        Size(term.right, width);
        break;
      default:
        throw std::runtime_error("cannot infer logic width");
    }
    term.width = width;
  }

  std::optional<Constant> Enumeration(const icl::LogicExpression& left,
                                      const icl::LogicExpression& right) const {
    if (left.operation != "reference" || right.operation != "reference") {
      return std::nullopt;
    }
    const auto lhs = netlist_.ExpressionAt(node_.scope, left.span);
    const auto rhs = netlist_.ExpressionAt(node_.scope, right.span);
    const auto& scope = netlist_.scopes[node_.scope];
    const auto& tokens = scope.definition.document->driver().unit().tokens();
    if (rhs.end != rhs.begin + 1) return std::nullopt;
    for (size_t i = lhs.begin; i < lhs.end; ++i) {
      if (tokens[i].text == "[") {
        throw std::runtime_error("enum comparison cannot select a subrange");
      }
    }
    const auto bits = netlist_.Resolve(lhs);
    if (bits.empty()) return std::nullopt;
    const auto& source = netlist_.nodes.at(bits[0].node);
    const auto ref = source.properties.find("RefEnum");
    if (ref == source.properties.end()) return std::nullopt;
    const auto& source_scope = netlist_.scopes[source.scope];
    const auto& source_tokens =
        source_scope.definition.document->driver().unit().tokens();
    const auto name = source_tokens[ref->second.begin].text;
    const auto& table = netlist_.NodeAt(
        source_scope.path.empty() ? name : source_scope.path + "." + name);
    if (table.model != ModelKind::kEnum) return std::nullopt;
    const auto found = table.constants.find(tokens[rhs.begin].text);
    return found == table.constants.end()
               ? std::nullopt
               : std::optional<Constant>(found->second);
  }

  size_t Build(const icl::LogicExpression& expression, size_t depth,
               bool comparison_value = false) {
    if (depth > 128) {
      throw std::runtime_error("LogicSignal expression depth limit exceeded");
    }
    LogicInstruction term;
    const auto& op = expression.operation;
    if (!expression.left) {
      const auto source = netlist_.ExpressionAt(node_.scope, expression.span);
      if (op == "reference") {
        term.operation = LogicOperation::kSignal;
        term.signals = netlist_.Resolve(source, 0, false, remaining_bits_);
        for (const auto& bit : term.signals) {
          if (bit.node == std::numeric_limits<size_t>::max()) continue;
          const auto& target = netlist_.nodes[bit.node];
          const auto& kind = target.kind;
          if (kind != "DataInPort" && kind != "SelectPort" &&
              kind != "DataOutPort" && kind != "ToSelectPort" &&
              kind != "ScanRegister" && kind != "DataRegister" &&
              kind != "LogicSignal" && kind != "DataMux" &&
              kind != "OneHotDataGroup" && kind != "Alias") {
            throw std::runtime_error(target.name + " is not a data signal");
          }
        }
        term.width = term.signals.size();
      } else {
        term.literal = netlist_.Value(source);
        if (term.literal.is_string) {
          throw std::runtime_error("string parameter used in LogicSignal");
        }
        if (!comparison_value &&
            term.literal.data.find('x') != std::string::npos) {
          throw std::runtime_error(
              "unknown bits are only allowed in a comparison value");
        }
        term.width = term.literal.sized ? term.literal.data.size() : 0;
      }
    } else {
      term.left = Build(*expression.left, depth + 1, comparison_value);
      if (expression.right) {
        if ((op == "==" || op == "!=") &&
            expression.right->operation == "reference") {
          const auto value = Enumeration(*expression.left, *expression.right);
          if (!value) {
            throw std::runtime_error(
                "comparison requires a numeric value or matching RefEnum "
                "symbol");
          }
          term.right = node_.logic.size();
          Append(
              {LogicOperation::kLiteral, 0, 0, value->data.size(), *value, {}});
        } else {
          term.right = Build(*expression.right, depth + 1,
                             comparison_value || op == "==" || op == "!=");
        }
        const size_t left_width = node_.logic[term.left].width,
                     right_width = node_.logic[term.right].width;
        if (op == ",") {
          term.operation = LogicOperation::kConcat;
          term.width = left_width && right_width ? left_width + right_width : 0;
        } else if (op == "==" || op == "!=") {
          term.operation =
              op == "==" ? LogicOperation::kEqual : LogicOperation::kNotEqual;
          const size_t width = left_width ? left_width : right_width;
          Size(term.left, width);
          Size(term.right, width);
          term.width = 1;
        } else {
          term.operation = op == "&" || op == "&&"   ? LogicOperation::kAnd
                           : op == "|" || op == "||" ? LogicOperation::kOr
                                                     : LogicOperation::kXor;
          term.width = op == "&&" || op == "||" ? 1
                       : left_width             ? left_width
                                                : right_width;
          if (term.width) {
            Size(term.left, term.width);
            Size(term.right, term.width);
          }
        }
      } else {
        if (op == "~") {
          term.operation = LogicOperation::kInvert;
          term.width = node_.logic[term.left].width;
          if (comparison_value && !term.width) {
            throw std::runtime_error(
                "cannot invert an unsized comparison value");
          }
        } else if (op == "!") {
          term.operation = LogicOperation::kNot;
          Size(term.left, 1);
          term.width = 1;
        } else {
          term.operation = op == "&"   ? LogicOperation::kReduceAnd
                           : op == "|" ? LogicOperation::kReduceOr
                                       : LogicOperation::kReduceXor;
          Size(term.left, node_.logic[term.left].width);
          term.width = 1;
        }
      }
    }
    Append(std::move(term));
    return node_.logic.size() - 1;
  }

  const Netlist& netlist_;
  NetlistNode& node_;
  size_t& remaining_bits_;
};

char Invert(char value) {
  return value == '0' ? '1' : value == '1' ? '0' : 'x';
}

char Bitwise(LogicOperation op, char a, char b) {
  if (op == LogicOperation::kAnd || op == LogicOperation::kReduceAnd) {
    return a == '0' || b == '0' ? '0' : a == '1' && b == '1' ? '1' : 'x';
  }
  if (op == LogicOperation::kOr || op == LogicOperation::kReduceOr) {
    return a == '1' || b == '1' ? '1' : a == '0' && b == '0' ? '0' : 'x';
  }
  return a == 'x' || b == 'x' ? 'x' : a == b ? '0' : '1';
}
}  // namespace

void CompileLogic(Netlist* netlist, size_t node, size_t* remaining_bits) {
  Compiler(netlist, node, remaining_bits).Run();
}

char EvaluateLogic(const NetlistNode& node,
                   const std::function<char(const BitReference&)>& value) {
  std::vector<std::string> results;
  for (const auto& term : node.logic) {
    std::string result;
    switch (term.operation) {
      case LogicOperation::kLiteral:
        result = term.literal.data;
        break;
      case LogicOperation::kSignal:
        for (const auto& bit : term.signals) {
          char v = bit.node == std::numeric_limits<size_t>::max() ? bit.constant
                                                                  : value(bit);
          if (v != '0' && v != '1' && v != 'x') {
            throw std::runtime_error("invalid logic input state");
          }
          result += bit.inverted ? Invert(v) : v;
        }
        break;
      case LogicOperation::kConcat:
        result = results.at(term.left) + results.at(term.right);
        break;
      case LogicOperation::kInvert:
      case LogicOperation::kNot:
        for (char bit : results.at(term.left)) result += Invert(bit);
        break;
      case LogicOperation::kReduceAnd:
      case LogicOperation::kReduceOr:
      case LogicOperation::kReduceXor: {
        char reduced = term.operation == LogicOperation::kReduceAnd ? '1' : '0';
        for (char bit : results.at(term.left)) {
          reduced = Bitwise(term.operation, reduced, bit);
        }
        result += reduced;
        break;
      }
      case LogicOperation::kEqual:
      case LogicOperation::kNotEqual: {
        char equal = '1';
        const auto& left = results.at(term.left);
        const auto& right = results.at(term.right);
        for (size_t i = 0; i < left.size(); ++i) {
          if (right[i] == 'x') continue;
          if (left[i] == 'x') {
            equal = 'x';
          } else if (left[i] != right[i]) {
            equal = '0';
            break;
          }
        }
        result +=
            term.operation == LogicOperation::kEqual ? equal : Invert(equal);
        break;
      }
      default: {
        const auto& left = results.at(term.left);
        const auto& right = results.at(term.right);
        for (size_t i = 0; i < left.size(); ++i) {
          result += Bitwise(term.operation, left[i], right[i]);
        }
      }
    }
    results.push_back(std::move(result));
  }
  if (results.empty() || results.back().size() != 1) {
    throw std::runtime_error("invalid compiled LogicSignal");
  }
  return results.back()[0];
}
}  // namespace pattern
