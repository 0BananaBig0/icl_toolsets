// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pattern_tap.h"

#include <algorithm>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace pattern {
namespace {
constexpr size_t kMaxModelBytes = 16 * 1024 * 1024;
constexpr size_t kMaxActiveBits = 1024 * 1024;
struct SignalRange {
  bool indexed = false;
  int64_t left = 0, right = 0;
};
struct Signal : SignalRange {
  std::vector<std::vector<size_t>> instructions;
};
bool Identifier(const std::string& name) {
  const auto letter = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  };
  if (name.empty() || !letter(name[0])) return false;
  for (char c : name) {
    if (!letter(c) && !(c >= '0' && c <= '9') && c != '_') return false;
  }
  return true;
}
class Builder {
 public:
  explicit Builder(const AccessLink& link) : link_(link) {}
  TapInstructionModel Run() {
    if (!link_.bsdl) throw std::runtime_error("missing bound BSDL metadata");
    ReadSignalRanges();
    const auto input = Fresh("tdi");
    const auto reg = Fresh("INSTRUCTION");
    const auto width = std::to_string(link_.bsdl->instruction_width);
    Append("Module IclPatternInstruction {\nScanInPort " + input + ";\n");
    Append("ScanRegister " + reg + "[" +
           std::to_string(link_.bsdl->instruction_width - 1) +
           ":0] { ScanInSource " + input + "; CaptureSource " + width + "'b" +
           link_.bsdl->capture + "; }\n");
    std::map<std::string, std::string> decoders;
    std::vector<std::string> decoder_names;
    size_t remaining_bits = kMaxActiveBits;
    for (const auto& instruction : link_.instructions) {
      const auto name = Fresh("instruction_select");
      std::vector<std::string> alternatives;
      for (const auto& code : link_.bsdl->opcodes.at(instruction.name)) {
        alternatives.push_back(reg + " == " + width + "'b" + code);
      }
      Append("LogicSignal " + name + " { " + Or(std::move(alternatives)) +
             "; }\n");
      decoders.emplace(instruction.name, name);
      const size_t instruction_id = decoder_names.size();
      decoder_names.push_back(name);
      std::set<std::string> seen;
      for (const auto& signal : instruction.active_signals) {
        if (!seen.insert(signal.text).second) continue;
        const auto& range = ranges_.at(signal.text);
        auto& output = signals_.at(BaseName(signal.text));
        const auto lower = std::min(output.left, output.right);
        const auto first = std::min(range.left, range.right);
        const size_t width = std::max(range.left, range.right) - first + 1;
        if (width > remaining_bits) {
          throw std::runtime_error(signal.origin +
                                   ": ActiveSignal selection limit exceeded");
        }
        remaining_bits -= width;
        for (size_t offset = 0; offset < width; ++offset) {
          auto& selected = output.instructions[first - lower + offset];
          if (!selected.empty() && selected.back() == instruction_id) continue;
          selected.push_back(instruction_id);
        }
      }
    }
    std::map<std::vector<size_t>, std::string> shared_sources;
    for (const auto& entry : signals_) {
      const auto& signal = entry.second;
      std::string sources;
      for (size_t offset = 0; offset < signal.instructions.size(); ++offset) {
        const size_t index = signal.left > signal.right
                                 ? signal.instructions.size() - 1 - offset
                                 : offset;
        const auto& selected = signal.instructions[index];
        std::string source = "1'b0";
        if (selected.size() == 1) {
          source = decoder_names[selected.front()];
        } else if (!selected.empty()) {
          auto found = shared_sources.find(selected);
          if (found == shared_sources.end()) {
            std::vector<std::string> terms;
            for (size_t id : selected) terms.push_back(decoder_names[id]);
            source = Fresh("active_signal");
            Append("LogicSignal " + source + " { " + Or(std::move(terms)) +
                   "; }\n");
            found = shared_sources.emplace(selected, source).first;
          }
          source = found->second;
        }
        const size_t separator = sources.empty() ? 0 : 1;
        if (source.size() + separator > kMaxModelBytes - sources.size()) {
          throw std::runtime_error("ActiveSignal source text limit exceeded");
        }
        if (separator) sources += ",";
        sources += source;
      }
      const auto range = signal.indexed
                             ? "[" + std::to_string(signal.left) + ":" +
                                   std::to_string(signal.right) + "]"
                             : "";
      Append("DataOutPort " + entry.first + range + " { Source " + sources +
             "; }\n");
    }
    Append("}\n");
    Design design;
    design.AddIcl(link_.origin + " (instruction component)", text_);
    TapInstructionModel model;
    model.netlist = Elaborate(design, "IclPatternInstruction");
    model.instruction_register = model.netlist.node_ids.at(reg);
    model.scan_input = model.netlist.node_ids.at(input);
    for (const auto& decoder : decoders) {
      model.instruction_selects.emplace(
          decoder.first, model.netlist.node_ids.at(decoder.second));
    }
    return model;
  }

 private:
  static std::string BaseName(const std::string& text) {
    return text.substr(0, text.find('['));
  }
  void ReadSignalRanges() {
    // Batch range/parameter evaluation through the shared frontend, not a new
    // ActiveSignal expression parser. Temporary ports carry only their shape.
    Append("Module IclPatternSignalShapes {\n");
    std::map<std::string, std::string> origins;
    if (link_.module) {
      for (const auto* node : link_.module->ast->body()) {
        const auto* statement = dynamic_cast<const icl::AstStatement*>(node);
        if (!statement ||
            (statement->named_kind() != icl::NamedKind::kParameter &&
             statement->named_kind() != icl::NamedKind::kLocalParameter)) {
          continue;
        }
        names_.insert(statement->name());
        const auto range = link_.module->document->Range(node->span());
        Append(link_.module->document->source().substr(
                   range.begin, range.end - range.begin) +
               "\n");
      }
    }
    std::map<std::string, std::string> shape_names;
    for (const auto& instruction : link_.instructions) {
      for (const auto& signal : instruction.active_signals) {
        const auto base = BaseName(signal.text);
        if (!Identifier(base)) {
          throw std::runtime_error(signal.origin +
                                   ": invalid ActiveSignal name");
        }
        names_.insert(base);
        if (shape_names.count(signal.text)) continue;
        const auto name = Fresh("active_range");
        shape_names.emplace(signal.text, name);
        origins.emplace(name, signal.origin);
        Append("DataInPort " + name + signal.text.substr(base.size()) + ";\n");
      }
    }
    Append("}\n");
    Design design;
    design.AddIcl(link_.origin + " (ActiveSignal ranges)", text_);
    Netlist shapes;
    try {
      shapes = Elaborate(design, "IclPatternSignalShapes");
    } catch (const std::runtime_error& error) {
      std::string message = error.what();
      const auto& module = design.SelectTop("IclPatternSignalShapes");
      for (const auto& declaration : module.document->declarations()) {
        const auto origin = origins.find(declaration.name);
        if (origin == origins.end()) continue;
        const auto generated =
            module.Location(declaration.name_range.begin) + ":";
        const auto at = message.find(generated);
        if (at != std::string::npos) {
          message.replace(at, generated.size(), origin->second + ":");
          break;
        }
      }
      throw std::runtime_error(message);
    }
    size_t remaining_bits = kMaxActiveBits;
    for (const auto& instruction : link_.instructions) {
      for (const auto& signal : instruction.active_signals) {
        if (ranges_.count(signal.text)) continue;
        const auto& node = shapes.NodeAt(shape_names.at(signal.text));
        ranges_.emplace(signal.text,
                        SignalRange{node.indexed, node.left, node.right});
        const auto base = BaseName(signal.text);
        auto found = signals_.find(base);
        if (found == signals_.end()) {
          Signal output;
          output.indexed = node.indexed;
          output.left = node.left;
          output.right = node.right;
          signals_.emplace(base, std::move(output));
        } else {
          auto& output = found->second;
          if (output.indexed != node.indexed) {
            throw std::runtime_error(
                signal.origin +
                ": scalar/indexed ActiveSignal conflict: " + base);
          }
          const auto lower =
              std::min({output.left, output.right, node.left, node.right});
          const auto upper =
              std::max({output.left, output.right, node.left, node.right});
          const bool descending = output.left >= output.right;
          output.left = descending ? upper : lower;
          output.right = descending ? lower : upper;
        }
      }
    }
    for (auto& entry : signals_) {
      auto& signal = entry.second;
      const auto width =
          static_cast<size_t>(std::max(signal.left, signal.right) -
                              std::min(signal.left, signal.right)) +
          1;
      if (width > remaining_bits) {
        throw std::runtime_error("ActiveSignal width limit exceeded: " +
                                 entry.first);
      }
      remaining_bits -= width;
      signal.instructions.resize(width);
    }
    text_.clear();
  }
  std::string Fresh(const std::string& base) {
    auto name = base;
    while (!names_.insert(name).second) {
      name = base + "_" + std::to_string(next_name_++);
    }
    return name;
  }
  void Append(const std::string& text) {
    if (text.size() > kMaxModelBytes - text_.size()) {
      throw std::runtime_error("TAP instruction model text limit exceeded");
    }
    text_ += text;
  }
  static std::string Or(std::vector<std::string> terms) {
    if (terms.empty()) throw std::runtime_error("instruction has no opcode");
    while (terms.size() > 1) {
      std::vector<std::string> merged;
      for (size_t i = 0; i < terms.size(); i += 2) {
        if (i + 1 == terms.size()) {
          merged.push_back(std::move(terms[i]));
        } else {
          merged.push_back("(" + terms[i] + ") || (" + terms[i + 1] + ")");
        }
      }
      terms = std::move(merged);
    }
    return std::move(terms.front());
  }
  const AccessLink& link_;
  std::set<std::string> names_;
  std::map<std::string, SignalRange> ranges_;
  std::map<std::string, Signal> signals_;
  std::string text_;
  size_t next_name_ = 0;
};
}  // namespace
TapInstructionModel BuildTapInstructionModel(const AccessLink& link) {
  try {
    return Builder(link).Run();
  } catch (const std::exception& error) {
    throw std::runtime_error(link.origin + ": AccessLink " + link.name + ": " +
                             error.what());
  }
}
}  // namespace pattern
