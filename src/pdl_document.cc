// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#include "pdl_document.h"

#include <tcl.h>

#include <algorithm>
#include <climits>
#include <functional>
#include <set>
#include <sstream>
#include <string_view>

#include "icl_semantics.h"

namespace pdl {
namespace {

using TclCount = decltype(Tcl_Parse{}.numWords);

Tcl_Interp* ParserInterpreter() {
  // Tcl_ParseExpr uses interpreter stack storage. Never call Tcl_Init or Eval.
  static thread_local std::unique_ptr<Tcl_Interp, decltype(&Tcl_DeleteInterp)>
      interpreter(Tcl_CreateInterp(), Tcl_DeleteInterp);
  return interpreter.get();
}
struct CommandSpec {
  int minimum, maximum, level;
};
const std::map<std::string, CommandSpec>& Specs() {
  static const std::map<std::string, CommandSpec> kSpecs = {
      {"iPDLLevel", {1, 3, 0}},
      {"iProcsForModule", {1, 3, 0}},
      {"iProc", {3, 3, 0}},
      {"iUseProcNameSpace", {1, 1, 0}},
      {"iPrefix", {1, 1, 0}},
      {"iCall", {1, INT_MAX, 0}},
      {"iReset", {0, 1, 0}},
      {"iRead", {1, 2, 0}},
      {"iWrite", {2, 4, 0}},
      {"iScan", {2, 10, 0}},
      {"iOverrideScanInterface", {1, INT_MAX, 0}},
      {"iApply", {0, 1, 0}},
      {"iClock", {1, 1, 0}},
      {"iClockOverride", {1, 7, 0}},
      {"iRunLoop", {1, 3, 0}},
      {"iMerge", {1, 1, 0}},
      {"iTake", {0, 2, 0}},
      {"iRelease", {1, 1, 0}},
      {"iNote", {0, INT_MAX, 0}},
      {"iState", {2, 3, 0}},
      {"iGetReadData", {1, 4, 1}},
      {"iGetMiscompares", {1, 4, 1}},
      {"iGetStatus", {0, 1, 1}},
      {"iSetFail", {1, 2, 1}}};
  return kSpecs;
}

std::string Text(const std::string& source, ByteRange range) {
  return source.substr(range.begin, range.end - range.begin);
}

std::string NormalizeContinuations(std::string text) {
  size_t i = 0;
  while ((i = text.find("\\\r\n", i)) != std::string::npos) {
    // Tcl expects LF; retain byte offsets into the original CRLF spelling.
    text[i + 1] = '\n';
    text[i + 2] = ' ';
    i += 3;
  }
  return text;
}

std::string LiteralValue(std::string text) {
  std::string value;
  value.reserve(text.size());
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\\' && i + 1 < text.size()) {
      size_t end = i + 1;
      if (text[end] == '\r' && end + 1 < text.size()) ++end;
      if (text[end] == '\n') {
        while (end + 1 < text.size() &&
               (text[end + 1] == ' ' || text[end + 1] == '\t')) {
          ++end;
        }
        value += ' ';
        i = end;
        continue;
      }
      value += text[i++];
    }
    value += text[i];
  }
  return value;
}

std::vector<std::string> SplitList(const std::string& text, bool* valid) {
  TclCount count = 0;
  const char** values = nullptr;
  *valid = Tcl_SplitList(nullptr, text.c_str(), &count, &values) == TCL_OK;
  std::vector<std::string> result;
  if (*valid) {
    for (TclCount i = 0; i < count; ++i) result.emplace_back(values[i]);
    Tcl_Free(reinterpret_cast<char*>(values));
  }
  return result;
}

bool IsListSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ||
         c == '\v';
}

// Tcl's parser recursively descends substitutions before returning tokens.
// Walk their delimiters first, leaving syntax validation to Tcl itself.
size_t ExcessiveNesting(std::string_view text, int depth, bool expression) {
  enum class Mode { kScript, kExpression, kQuote, kArray };
  struct Frame {
    Mode mode;
    bool command_start = true, word_start = true;
  };
  std::vector<Frame> frames{{expression ? Mode::kExpression : Mode::kScript}};
  for (size_t i = 0; i < text.size(); ++i) {
    auto& frame = frames.back();
    const char c = text[i];
    const bool script = frame.mode == Mode::kScript;
    const bool expr = frame.mode == Mode::kExpression;
    if (c == '\\') {
      if (i + 1 < text.size()) {
        const bool continuation = text[i + 1] == '\n';
        ++i;
        if (continuation) {
          while (i + 1 < text.size() &&
                 (text[i + 1] == ' ' || text[i + 1] == '\t')) {
            ++i;
          }
          if (script) frame.word_start = true;
          continue;
        }
      }
      frame.command_start = frame.word_start = false;
      continue;
    }
    if (script && frame.command_start && c == '#') {
      while (i < text.size() && text[i] != '\n') {
        if (text[i] == '\\' && i + 1 < text.size()) ++i;
        ++i;
      }
      continue;
    }
    if (script && (IsListSpace(c) || c == ';')) {
      frame.word_start = true;
      if (c == '\n' || c == ';') frame.command_start = true;
      continue;
    }
    if (c == ']' && script && frames.size() > 1) {
      frames.pop_back();
      --depth;
      continue;
    }
    if (c == ')' && frame.mode == Mode::kArray) {
      frames.pop_back();
      --depth;
      continue;
    }
    if (c == '"' && frame.mode == Mode::kQuote) {
      frames.pop_back();
      continue;
    }
    const bool word_start = frame.word_start;
    frame.command_start = frame.word_start = false;
    if (c == '{' && (expr || (script && word_start))) {
      if (script && text.substr(i, 3) == "{*}" && i + 3 < text.size() &&
          !IsListSpace(text[i + 3]) && text[i + 3] != ';' &&
          text[i + 3] != ']') {
        i += 2;
        frame.word_start = true;
        continue;
      }
      int braces = 1;
      while (++i < text.size()) {
        if (text[i] == '\\' && i + 1 < text.size()) {
          ++i;
        } else if (text[i] == '{') {
          ++braces;
        } else if (text[i] == '}' && --braces == 0) {
          break;
        }
      }
    } else if (c == '"' && (expr || (script && word_start))) {
      frames.push_back({Mode::kQuote});
    } else if (c == '[') {
      if (++depth > 128) return i;
      frames.push_back({Mode::kScript});
    } else if (c == '$' && i + 1 < text.size()) {
      if (text[i + 1] == '{') {
        i = text.find('}', i + 2);
        if (i == std::string_view::npos) break;
        continue;
      }
      const size_t start = i + 1;
      size_t end = start;
      while (end < text.size()) {
        const unsigned char ch = text[end];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '_' || ch >= 128) {
          ++end;
        } else if (ch == ':' && end + 1 < text.size() && text[end + 1] == ':') {
          while (end < text.size() && text[end] == ':') ++end;
        } else {
          break;
        }
      }
      if (end > start && end < text.size() && text[end] == '(') {
        if (++depth > 128) return end;
        frames.push_back({Mode::kArray});
        i = end;
      } else {
        i = end - 1;
      }
    }
  }
  return std::string_view::npos;
}

// SplitList validates the list first; this walk only recovers original spans.
std::vector<ByteRange> ListSpans(const std::string& source, ByteRange range) {
  const auto continuation = [&](size_t i) -> size_t {
    if (source[i] != '\\' || i + 1 >= range.end) return 0;
    if (source[i + 1] == '\n') return 2;
    return i + 2 < range.end && source[i + 1] == '\r' && source[i + 2] == '\n'
               ? 3
               : 0;
  };
  std::vector<ByteRange> result;
  size_t i = range.begin;
  while (i < range.end) {
    if (const size_t length = continuation(i)) {
      i += length;
      continue;
    }
    if (IsListSpace(source[i])) {
      ++i;
      continue;
    }
    const size_t start = i;
    const bool braced = source[i] == '{', quoted = source[i] == '"';
    int depth = braced ? 1 : 0;
    if (braced || quoted) ++i;
    while (i < range.end) {
      const char c = source[i++];
      if (c == '\\' && i < range.end) {
        if (!braced && !quoted && continuation(i - 1)) {
          --i;
          break;
        }
        ++i;
        continue;
      }
      if (braced) {
        if (c == '{') ++depth;
        if (c == '}' && --depth == 0) break;
      } else if (quoted) {
        if (c == '"') break;
      } else if (IsListSpace(c)) {
        --i;
        break;
      }
    }
    result.push_back({start, i});
  }
  return result;
}

bool IsScalar(const std::string& text) {
  const auto alpha = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  };
  return !text.empty() && alpha(text[0]) &&
         std::all_of(text.begin(), text.end(), [&](char c) {
           return alpha(c) || (c >= '0' && c <= '9') || c == '_';
         });
}

bool IsNumber(const std::string& text) {
  if (text.empty()) return false;
  if (text[0] == '$') return IsScalar(text.substr(1));
  size_t start = 0;
  char base = 'd';
  if (text.size() > 1 && text[0] == '0' &&
      (text[1] == 'x' || text[1] == 'X' || text[1] == 'b' || text[1] == 'B')) {
    base = text[1] == 'x' || text[1] == 'X' ? 'h' : 'b';
    start = 2;
  }
  return start < text.size() &&
         std::all_of(
             text.begin() + start, text.end(),
             [&](char c) {
               if (base != 'd' && (c == 'x' || c == 'X')) return true;
               if (base == 'h' &&
                   ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                 return true;
               }
               return c >= '0' && c <= (base == 'b' ? '1' : '9');
             });
}

bool IsTime(const std::string& text) {
  size_t i = 0;
  const auto digits = [&]() {
    const size_t start = i;
    while (i < text.size() && text[i] >= '0' && text[i] <= '9') ++i;
    return i > start;
  };
  if (!digits()) return false;
  const bool decimal = i < text.size() && text[i] == '.';
  if (decimal) {
    ++i;
    if (!digits()) return false;
  }
  if (i < text.size() && text[i] == 'e') {
    if (!decimal) return false;
    ++i;
    if (i == text.size() || text[i++] != '-' || !digits()) return false;
  }
  static const std::set<std::string> kUnits = {"s",  "ms", "us", "ns",
                                               "ps", "fs", "as"};
  return kUnits.count(text.substr(i)) != 0;
}

bool IsVector(const std::string& text) {
  const auto colon = text.find(':');
  return colon == std::string::npos ? IsNumber(text)
                                    : IsNumber(text.substr(0, colon)) &&
                                          IsNumber(text.substr(colon + 1));
}

bool IsHierSignal(const std::string& text) {
  if (!text.empty() && text[0] == '$') return IsScalar(text.substr(1));
  size_t begin = 0;
  while (true) {
    const auto dot = text.find('.', begin);
    if (dot == std::string::npos) break;
    if (!IsScalar(text.substr(begin, dot - begin))) return false;
    begin = dot + 1;
  }
  const auto leaf = text.substr(begin);
  const auto open = leaf.find_first_of("([");
  if (open == std::string::npos) return IsScalar(leaf);
  return IsScalar(leaf.substr(0, open)) && leaf.size() > open + 2 &&
         leaf.back() == (leaf[open] == '(' ? ')' : ']') &&
         IsVector(leaf.substr(open + 1, leaf.size() - open - 2));
}

bool IsInstancePath(const std::string& text) {
  return text.find_first_of("([$)") == std::string::npos && IsHierSignal(text);
}

bool IsQualifiedScalar(const std::string& text) {
  const auto separator = text.find("::");
  return separator == std::string::npos
             ? IsScalar(text)
             : IsScalar(text.substr(0, separator)) &&
                   IsScalar(text.substr(separator + 2));
}

bool IsProcedurePath(const std::string& text) {
  const auto dot = text.rfind('.');
  return dot == std::string::npos ? IsQualifiedScalar(text)
                                  : IsInstancePath(text.substr(0, dot)) &&
                                        IsQualifiedScalar(text.substr(dot + 1));
}

std::string Qualify(const std::string& current, const std::string& name) {
  if (name.compare(0, 2, "::") == 0) return name.substr(2);
  return current.empty() ? name : current + "::" + name;
}

std::pair<std::string, std::string> SplitQualified(const std::string& name) {
  const auto separator = name.rfind("::");
  return separator == std::string::npos
             ? std::make_pair(std::string(), name)
             : std::make_pair(name.substr(0, separator),
                              name.substr(separator + 2));
}

std::string ArrayName(const std::string& name) {
  return !name.empty() && name.back() == ')' ? name.substr(0, name.find('('))
                                             : name;
}

bool HasExpandedArguments(const Command& command) {
  return std::any_of(command.words.begin(), command.words.end(),
                     [](const Word& word) { return word.expanded; });
}

std::set<size_t> BodyWords(const Command& command,
                           std::set<size_t>* expressions, std::string* error) {
  if (HasExpandedArguments(command)) return {};
  const auto& words = command.words;
  const std::string& name = words.front().value;
  const size_t size = words.size();
  const auto invalid = [&]() {
    *error = "invalid Tcl command structure for " + name;
    return std::set<size_t>{};
  };
  static const std::map<std::string, std::pair<size_t, size_t>> kWordCounts = {
      {"proc", {4, 4}},       {"set", {2, 3}},
      {"while", {3, 3}},      {"for", {5, 5}},
      {"catch", {2, 4}},      {"if", {3, INT_MAX}},
      {"try", {2, INT_MAX}},  {"switch", {3, INT_MAX}},
      {"expr", {2, INT_MAX}}, {"foreach", {4, INT_MAX}},
      {"lmap", {4, INT_MAX}}};
  const auto count = kWordCounts.find(name);
  if (count != kWordCounts.end() &&
      (size < count->second.first || size > count->second.second)) {
    return invalid();
  }
  if ((name == "iProc" || name == "proc") && size == 4) return {3};
  if (name == "foreach" || name == "lmap") {
    if (size % 2 != 0) return invalid();
    for (size_t i = 1; i + 1 < size; ++i) {
      if (!words[i].literal) continue;
      bool valid = false;
      const auto variables = SplitList(words[i].value, &valid);
      if (!valid || (i % 2 != 0 && variables.empty())) return invalid();
    }
    return {size - 1};
  }
  if (name == "for" && size == 5) return {1, 3, 4};
  if (name == "while" && size == 3) return {2};
  if (name == "catch" && size >= 2) return {1};
  if (name == "try" && size >= 2) {
    std::set<size_t> result{1};
    bool fallthrough = false;
    for (size_t i = 2; i < size; i += 4) {
      if (!words[i].literal) return result;
      const auto& handler = words[i].value;
      const auto matches = [&](const std::string& keyword) {
        return !handler.empty() &&
               keyword.compare(0, handler.size(), handler) == 0;
      };
      if (matches("finally")) {
        if (i + 2 != size || fallthrough) return invalid();
        result.insert(i + 1);
        return result;
      }
      if ((!matches("on") && !matches("trap")) || i + 3 >= size) {
        return invalid();
      }
      if (matches("on") && words[i + 1].literal) {
        static const std::set<std::string> kCodes = {"ok", "error", "return",
                                                     "break", "continue"};
        int code = 0;
        if (!kCodes.count(words[i + 1].value) &&
            Tcl_GetInt(ParserInterpreter(), words[i + 1].value.c_str(),
                       &code) != TCL_OK) {
          return invalid();
        }
      }
      for (size_t j : {i + 1, i + 2}) {
        if (j == i + 1 && !matches("trap")) continue;
        if (words[j].literal) {
          bool valid = false;
          SplitList(words[j].value, &valid);
          if (!valid) return invalid();
        }
      }
      fallthrough = words[i + 3].literal && words[i + 3].value == "-";
      if (!fallthrough) result.insert(i + 3);
    }
    if (fallthrough) return invalid();
    return result;
  }
  if (name == "namespace" && size == 4 && words[1].value == "eval") return {3};
  if (name == "if" && size >= 3) {
    std::set<size_t> result;
    size_t test = 1;
    while (true) {
      if (test + 1 >= size) return invalid();
      expressions->insert(test);
      size_t i = test + 1;
      // A dynamic word may be the optional "then" or the body itself.
      if (!words[i].literal) return result;
      if (words[i].value == "then") ++i;
      if (i >= size) return invalid();
      result.insert(i++);
      if (i == size || !words[i].literal) return result;
      if (words[i].value == "elseif") {
        test = i + 1;
        continue;
      }
      if (words[i].value == "else") ++i;
      if (i + 1 != size) return invalid();
      result.insert(i);
      return result;
    }
  }
  return {};
}

std::string ParseError(int type) {
  switch (type) {
    case TCL_PARSE_MISSING_BRACE:
      return "missing close brace";
    case TCL_PARSE_MISSING_BRACKET:
      return "missing close bracket";
    case TCL_PARSE_MISSING_PAREN:
      return "missing close parenthesis";
    case TCL_PARSE_MISSING_QUOTE:
      return "missing close quote";
    case TCL_PARSE_QUOTE_EXTRA:
      return "extra characters after close quote";
    case TCL_PARSE_BRACE_EXTRA:
      return "extra characters after close brace";
    default:
      return "invalid Tcl/PDL syntax";
  }
}

std::string ProcedureKey(const std::string& binding, const std::string& ns,
                         const std::string& name) {
  return binding + '\n' + ns + '\n' + name;
}

void Render(const std::string& source, const std::vector<Command>& commands,
            int depth, int width, std::string* output) {
  const std::string indent(depth * width, ' ');
  for (const auto& command : commands) {
    *output += indent;
    if (!command.comment.empty()) {
      *output += command.comment;
    } else if (HasExpandedArguments(command)) {
      auto range = command.range;
      while (range.end > range.begin &&
             (source[range.end - 1] == '\n' || source[range.end - 1] == '\r')) {
        --range.end;
      }
      *output += Text(source, range);
    } else {
      for (size_t i = 0; i < command.words.size(); ++i) {
        if (i) *output += ' ';
        const auto& word = command.words[i];
        if (word.body) {
          *output += "{\n";
          Render(source, word.script, depth + 1, width, output);
          *output += indent + '}';
        } else {
          *output += Text(source, word.range);
        }
      }
    }
    *output += '\n';
  }
}

}  // namespace

int ClampIndentWidth(int requested) {
  return requested < 1 ? 2 : std::min(requested, 16);
}

Document::Document(std::string source, bool is_icl)
    : source_(std::move(source)), is_icl_(is_icl) {
  for (size_t i = 0; i < source_.size(); ++i) {
    if (source_[i] == '\n') line_starts_.push_back(i + 1);
  }
  if (source_.size() > INT_MAX || source_.find('\0') != std::string::npos) {
    Error({}, "PDL input is too large or contains NUL bytes");
    return;
  }
  if (is_icl) {
    const icl::ParsedDocument icl_document(source_);
    for (const auto& item : icl_document.declarations()) {
      const auto qualified = [](const std::string& ns,
                                const std::string& name) {
        return ns.empty() ? name : ns + "::" + name;
      };
      Declare({item.name,
               item.kind,
               item.detail,
               "",
               item.name_space,
               {item.name_range.begin, item.name_range.end},
               {item.range.begin, item.range.end},
               item.module,
               qualified(item.target_namespace, item.target_name)});
    }
    return;
  }
  if (source_.find("\\\r\n") != std::string::npos) {
    parser_source_ = NormalizeContinuations(source_);
  }
  commands_ = Parse(0, source_.size(), 0);
  Analyze(&commands_, {}, {0, source_.size()});
}

void Document::Error(ByteRange range, std::string message) {
  if (!ok()) return;
  error_range_ = range;
  error_message_ = std::move(message);
}

int Document::error_line() const {
  return static_cast<int>(std::upper_bound(line_starts_.begin(),
                                           line_starts_.end(),
                                           error_range_.begin) -
                          line_starts_.begin());
}

const char* Document::ParserData() const {
  return parser_source_.empty() ? source_.data() : parser_source_.data();
}

std::vector<Command> Document::Parse(size_t begin, size_t end, int depth) {
  std::vector<Command> commands;
  if (depth > 128) {
    Error({begin, begin}, "script nesting exceeds 128 levels");
    return commands;
  }
  const auto excessive = ExcessiveNesting(
      std::string_view(ParserData() + begin, end - begin), depth, false);
  if (excessive != std::string_view::npos) {
    Error({begin + excessive, begin + excessive + 1},
          "script nesting exceeds 128 levels");
    return commands;
  }
  while (begin < end) {
    Tcl_Parse parsed{};
    const int result =
        Tcl_ParseCommand(nullptr, ParserData() + begin,
                         static_cast<TclCount>(end - begin), 0, &parsed);
    if (parsed.commentStart && parsed.commentSize) {
      const size_t start = parsed.commentStart - ParserData();
      std::string comment = source_.substr(start, parsed.commentSize);
      while (!comment.empty() &&
             (comment.back() == '\n' || comment.back() == '\r')) {
        comment.pop_back();
      }
      commands.push_back(
          {{start, start + static_cast<size_t>(parsed.commentSize)},
           std::move(comment),
           {}});
    }
    const size_t start =
        parsed.commandStart ? parsed.commandStart - ParserData() : begin;
    Command command;
    command.range = {
        start, std::min(end, start + static_cast<size_t>(parsed.commandSize))};
    size_t token_end = start;
    bool static_expansion = false;
    for (TclCount i = 0; i < parsed.numTokens;) {
      const auto& token = parsed.tokenPtr[i];
      Word word;
      word.range = {
          static_cast<size_t>(token.start - ParserData()),
          static_cast<size_t>(token.start - ParserData() + token.size)};
      // Tcl folds constant {*} lists into tokens, omitting the source prefix.
      static_expansion |=
          Text(source_, {token_end, word.range.begin}).find("{*}") !=
          std::string::npos;
      token_end = word.range.end;
      word.content = word.range;
      const std::string raw = Text(source_, word.range);
      const bool braced =
          raw.size() >= 2 && raw.front() == '{' && raw.back() == '}';
      const bool quoted =
          raw.size() >= 2 && raw.front() == '"' && raw.back() == '"';
      if (braced || quoted) {
        ++word.content.begin;
        --word.content.end;
      }
      word.expanded = token.type == TCL_TOKEN_EXPAND_WORD;
      // A continuation changes whitespace, not whether a word is constant.
      word.literal =
          !word.expanded &&
          (braced || std::all_of(parsed.tokenPtr + i + 1,
                                 parsed.tokenPtr + i + 1 + token.numComponents,
                                 [](const Tcl_Token& part) {
                                   return part.type == TCL_TOKEN_TEXT ||
                                          (part.type == TCL_TOKEN_BS &&
                                           part.size >= 2 &&
                                           part.start[1] == '\n');
                                 }));
      word.value =
          word.literal ? LiteralValue(Text(source_, word.content)) : "";
      bool saw_vector = false;
      for (TclCount j = i + 1; j <= i + token.numComponents; ++j) {
        const auto& part = parsed.tokenPtr[j];
        const size_t offset = part.start - ParserData();
        if (part.type == TCL_TOKEN_VARIABLE && j + 1 < parsed.numTokens) {
          const auto& name = parsed.tokenPtr[j + 1];
          word.variables.push_back(
              {{static_cast<size_t>(name.start - ParserData()),
                static_cast<size_t>(name.start - ParserData() + name.size)},
               std::string(name.start, name.size)});
        }
        if (part.type == TCL_TOKEN_COMMAND && part.size >= 2) {
          const auto inside = source_.substr(offset + 1, part.size - 2);
          if (offset > word.range.begin && IsVector(inside)) {
            saw_vector = true;
            for (size_t k = 0; k < inside.size(); ++k) {
              if (inside[k] != '$') continue;
              const size_t first = ++k;
              while (k < inside.size() && inside[k] != ':') ++k;
              word.variables.push_back({{offset + 1 + first, offset + 1 + k},
                                        inside.substr(first, k - first)});
            }
            continue;
          }
          word.command_substitution = true;
          auto nested = Parse(offset + 1, offset + part.size - 1, depth + 1);
          word.substitutions.insert(word.substitutions.end(),
                                    std::make_move_iterator(nested.begin()),
                                    std::make_move_iterator(nested.end()));
        }
      }
      if (saw_vector && word.variables.empty() && !word.command_substitution) {
        word.literal = true;
        word.value = Text(source_, word.content);
      }
      command.words.push_back(std::move(word));
      i += token.numComponents + 1;
    }
    static_expansion |=
        Text(source_, {token_end, command.range.end}).find("{*}") !=
        std::string::npos;
    if (static_expansion) {
      if (command.words.empty()) {
        command.words.emplace_back();
        command.words.front().range = command.range;
      }
      command.words.front().expanded = true;
    }
    std::set<size_t> expressions;
    std::string structure_error;
    const auto bodies =
        command.words.empty() || result != TCL_OK
            ? std::set<size_t>{}
            : BodyWords(command, &expressions, &structure_error);
    if (!structure_error.empty()) {
      Error(command.words[0].range, structure_error);
    }
    for (const size_t i : bodies) {
      auto& word = command.words[i];
      if (word.literal) {
        word.body = source_[word.range.begin] == '{';
        word.script = Parse(word.content.begin, word.content.end, depth + 1);
      }
    }
    if (result == TCL_OK && !command.words.empty() &&
        !HasExpandedArguments(command)) {
      const auto& name = command.words[0].value;
      if (name == "while" && command.words.size() > 1) {
        expressions.insert(1);
      }
      if (name == "expr" && command.words.size() == 2 &&
          command.words[1].literal) {
        ParseExpression(&command.words[1], depth);
      } else if (name == "expr" && command.words.size() > 2 &&
                 std::all_of(command.words.begin() + 1, command.words.end(),
                             [](const Word& word) { return word.literal; })) {
        std::string text;
        for (size_t i = 1; i < command.words.size(); ++i) {
          if (i > 1) text += ' ';
          text += command.words[i].value;
        }
        if (ExcessiveNesting(text, depth, true) != std::string_view::npos) {
          Error(command.range, "script nesting exceeds 128 levels");
        } else {
          Tcl_Parse expression{};
          if (Tcl_ParseExpr(ParserInterpreter(), text.data(),
                            static_cast<TclCount>(text.size()),
                            &expression) != TCL_OK) {
            Error(command.range, "invalid Tcl expression");
          }
          Tcl_FreeParse(&expression);
        }
      }
      if (name == "for" && command.words.size() == 5) expressions.insert(2);
      for (const size_t i : expressions) {
        auto& word = command.words[i];
        if (word.literal && source_[word.range.begin] == '{') {
          ParseExpression(&word, depth);
        }
      }
      if (name == "switch" && command.words.size() >= 3) {
        size_t i = 1;
        bool known_options = true;
        bool has_mode = false;
        bool regexp = false;
        bool requires_regexp = false;
        while (i < command.words.size()) {
          if (!command.words[i].literal) {
            // A dynamic first argument may introduce options; preserve its
            // arguments.
            known_options = false;
            break;
          }
          auto option = command.words[i].value;
          if (option.empty() || option[0] != '-') break;
          static const std::vector<std::string> kSwitchOptions = {
              "--",      "-exact",    "-glob",    "-regexp",
              "-nocase", "-matchvar", "-indexvar"};
          std::string matched;
          int matches = 0;
          for (const auto& candidate : kSwitchOptions) {
            if (candidate.compare(0, option.size(), option) == 0) {
              matched = candidate;
              ++matches;
            }
          }
          if (matches != 1) {
            Error(command.words[i].range, "invalid or ambiguous switch option");
            known_options = false;
            break;
          }
          option = matched;
          ++i;
          if (option == "--") break;
          if (option == "-exact" || option == "-glob" || option == "-regexp") {
            if (has_mode) {
              Error(command.words[i - 1].range,
                    "switch permits only one match mode");
            }
            has_mode = true;
            regexp = option == "-regexp";
          }
          if (option == "-matchvar" || option == "-indexvar") {
            requires_regexp = true;
            if (i == command.words.size()) {
              Error(command.words[i - 1].range,
                    "switch option requires a variable name");
            }
            ++i;
          }
        }
        if (known_options && requires_regexp && !regexp) {
          Error(command.words[0].range,
                "switch match/index variables require -regexp");
        }
        if (!known_options) i = command.words.size();
        if (i < command.words.size()) ++i;  // Skip the switched value.
        if (known_options && i >= command.words.size()) {
          Error(command.words[0].range,
                "switch requires a value and pattern/body pairs");
        }
        if (i + 1 == command.words.size()) {
          auto& list = command.words[i];
          if (list.literal) {
            bool valid = false;
            const auto values = SplitList(list.value, &valid);
            if (!valid || values.empty() || values.size() % 2 != 0 ||
                values.back() == "-") {
              Error(list.range, "switch expects pattern/body pairs");
            } else {
              const auto spans = ListSpans(source_, list.content);
              for (size_t k = 1; k < spans.size(); k += 2) {
                auto body = spans[k];
                const char delimiter = source_[body.begin];
                if (delimiter == '{' || delimiter == '"') {
                  ++body.begin;
                  --body.end;
                }
                // Escaped list elements need decoded-to-source mapping.
                if (delimiter != '{' &&
                    Text(source_, body).find('\\') != std::string::npos) {
                  continue;
                }
                if (Text(source_, body) == "-") continue;
                auto script = Parse(body.begin, body.end, depth + 1);
                list.substitutions.insert(
                    list.substitutions.end(),
                    std::make_move_iterator(script.begin()),
                    std::make_move_iterator(script.end()));
              }
            }
          }
        } else {
          if (known_options && i < command.words.size() &&
              ((command.words.size() - i) % 2 != 0 ||
               (command.words.back().literal &&
                command.words.back().value == "-"))) {
            Error(command.words[0].range, "switch expects pattern/body pairs");
          }
          for (size_t j = i + 1; j < command.words.size(); j += 2) {
            auto& word = command.words[j];
            if (word.literal && word.value != "-") {
              word.body = source_[word.range.begin] == '{';
              word.script =
                  Parse(word.content.begin, word.content.end, depth + 1);
            }
          }
        }
      }
    }
    if (result != TCL_OK) {
      const size_t error = parsed.incomplete ? end : start;
      Error({error, std::min(end, error + 1)}, ParseError(parsed.errorType));
    }
    const size_t next = start + static_cast<size_t>(parsed.commandSize);
    Tcl_FreeParse(&parsed);
    if (!command.words.empty()) commands.push_back(std::move(command));
    if (result != TCL_OK) break;
    if (next <= begin) break;
    begin = next;
  }
  return commands;
}

void Document::ParseExpression(Word* word, int depth) {
  const auto excessive = ExcessiveNesting(
      std::string_view(ParserData() + word->content.begin,
                       word->content.end - word->content.begin),
      depth, true);
  if (excessive != std::string_view::npos) {
    const size_t offset = word->content.begin + excessive;
    Error({offset, offset + 1}, "script nesting exceeds 128 levels");
    return;
  }
  Tcl_Parse expression{};
  const int status = Tcl_ParseExpr(
      ParserInterpreter(), ParserData() + word->content.begin,
      static_cast<TclCount>(word->content.end - word->content.begin),
      &expression);
  if (status != TCL_OK) Error(word->content, "invalid Tcl expression");
  for (TclCount i = 0; i < expression.numTokens; ++i) {
    const auto& token = expression.tokenPtr[i];
    if (token.type == TCL_TOKEN_VARIABLE && i + 1 < expression.numTokens) {
      const auto& name = expression.tokenPtr[i + 1];
      word->variables.push_back(
          {{static_cast<size_t>(name.start - ParserData()),
            static_cast<size_t>(name.start - ParserData() + name.size)},
           std::string(name.start, name.size)});
    } else if (token.type == TCL_TOKEN_COMMAND && token.size >= 2) {
      const size_t begin = token.start - ParserData() + 1;
      auto nested = Parse(begin, begin + token.size - 2, depth + 1);
      word->substitutions.insert(word->substitutions.end(),
                                 std::make_move_iterator(nested.begin()),
                                 std::make_move_iterator(nested.end()));
    }
  }
  Tcl_FreeParse(&expression);
}

void Document::Declare(Declaration declaration) {
  const int index = static_cast<int>(declarations_.size());
  if (declaration.kind == "Variable") {
    const auto found = variables_.emplace(
        std::make_tuple(declaration.module, declaration.name_space,
                        declaration.name),
        index);
    if (!found.second) {
      uses_.push_back({declaration.name_range, declaration.name,
                       declaration.kind, declaration.binding,
                       declaration.name_space, declaration.module, -1});
      return;
    }
  }
  uses_.push_back({declaration.name_range, declaration.name, declaration.kind,
                   declaration.binding, declaration.name_space,
                   declaration.module, index});
  declarations_.push_back(std::move(declaration));
}

void Document::Analyze(std::vector<Command>* commands, Context context,
                       ByteRange scope) {
  contexts_.push_back({scope, context});
  bool merging = false;
  ByteRange merge_range;
  for (auto& command : *commands) {
    if (command.words.empty()) continue;
    auto& words = command.words;
    const auto& name = words[0].value;
    const auto spec = Specs().find(name);
    if (context.level == 0) {
      for (const auto& word : words) {
        if (word.command_substitution) {
          Error(word.range,
                "command substitution is not permitted in PDL level-0");
        }
      }
    }
    if (HasExpandedArguments(command)) {
      if (context.level == 0) {
        Error(words[0].range,
              "argument expansion is not permitted in PDL level-0");
      }
      if (name == "iProcsForModule") {
        context.has_binding = true;
        context.binding.clear();
        context.namespace_known = false;
      } else if (name == "iPrefix") {
        context.prefix_known = false;
      } else if (name == "iUseProcNameSpace") {
        context.call_namespace_known = false;
      } else if (name == "iPDLLevel") {
        context.level = -1;
      }
      contexts_.push_back({{command.range.begin, scope.end}, context});
      AnalyzeWords(&words, context, context);
      continue;
    }
    if (context.level == 0 &&
        (spec == Specs().end() || spec->second.level != 0)) {
      Error(words[0].range, "command is not permitted in PDL level-0: " + name);
    }
    if (spec != Specs().end()) {
      const int count = static_cast<int>(words.size()) - 1;
      if (count < spec->second.minimum || count > spec->second.maximum) {
        Error(words[0].range, "wrong number of arguments for " + name);
      }
    }
    const auto literal = [&](size_t i) {
      if (i >= words.size()) return std::string();
      return context.level == 0 && !words[i].literal
                 ? Text(source_, words[i].content)
                 : words[i].value;
    };
    const auto check_identifier = [&](size_t index,
                                      bool (*valid)(const std::string&),
                                      bool argument_ref = false) {
      if (index < words.size() &&
          (words[index].literal || context.level == 0)) {
        const auto value = literal(index);
        if (!valid(value) && !(argument_ref && !value.empty() &&
                               value[0] == '$' && IsNumber(value))) {
          Error(words[index].range, "invalid PDL identifier for " + name);
        }
      }
    };
    if (context.level == 0 && context.owner >= 0 &&
        (name == "iProc" || name == "iProcsForModule" || name == "iPDLLevel")) {
      Error(words[0].range,
            "declaration is not permitted inside a level-0 iProc");
    }
    if (name == "iProcsForModule") check_identifier(1, IsQualifiedScalar);
    if (name == "iProc") check_identifier(1, IsScalar);
    if (name == "iProc" && context.level == 0 && words.size() == 4) {
      for (size_t i : {2U, 3U}) {
        if (source_[words[i].range.begin] != '{') {
          Error(words[i].range,
                "level-0 iProc arguments and body require braces");
        }
      }
    }
    if (name == "iCall") {
      check_identifier(1, IsProcedurePath);
      if (context.level == 0) {
        for (size_t i = 2; i < words.size(); ++i) {
          if (!IsNumber(literal(i)) && !IsHierSignal(literal(i))) {
            Error(words[i].range, "invalid PDL argument for iCall");
          }
        }
      }
    }
    if (name == "iPrefix" && literal(1) != "-") {
      check_identifier(1, IsInstancePath);
    }
    if (name == "iUseProcNameSpace" && literal(1) != "-") {
      check_identifier(1, IsScalar);
    }
    if (name == "iPDLLevel") {
      if (words.size() < 2 ||
          ((words[1].literal || context.level == 0) && literal(1) != "0" &&
           literal(1) != "1") ||
          (words.size() != 2 && (words.size() != 4 ||
                                 ((words[2].literal || context.level == 0) &&
                                  literal(2) != "-version") ||
                                 ((words[3].literal || context.level == 0) &&
                                  literal(3) != "STD_1687_2014")))) {
        Error(words[0].range, "expected iPDLLevel 0|1 -version STD_1687_2014");
      } else {
        context.level = !words[1].literal ? -1 : (literal(1) == "0" ? 0 : 1);
      }
    } else if (name == "iProcsForModule" && words.size() >= 2) {
      context.has_binding = !words[1].literal || !literal(1).empty();
      context.binding = literal(1);
      if (!context.has_binding) {
        Error(words[1].range, "iProcsForModule requires a module name");
      }
      context.name_space.clear();
      context.namespace_known = true;
      if (words.size() == 4 && ((!words[2].literal && context.level != 0) ||
                                literal(2) == "-iProcNameSpace")) {
        check_identifier(3, IsScalar);
        context.name_space = literal(3);
        context.namespace_known = words[2].literal && words[3].literal;
      } else if (words.size() != 2) {
        Error(words[0].range,
              "expected iProcsForModule module [-iProcNameSpace namespace]");
      }
      uses_.push_back(
          {words[1].content, literal(1), "IclModule", "", "", -1, -1, ""});
    } else if (name == "iUseProcNameSpace") {
      context.call_namespace = literal(1) == "-" ? "" : literal(1);
      context.call_namespace_known = words.size() > 1 && words[1].literal;
    } else if (name == "iPrefix") {
      context.prefix = literal(1) == "-" ? "" : literal(1);
      context.prefix_known = words.size() > 1 && words[1].literal;
    }
    static const std::map<std::string, std::set<std::string>> kOptions = {
        {"iApply", {"-together", "-end_in_pause"}},
        {"iReset", {"-sync"}},
        {"iGetStatus", {"-clear"}},
        {"iMerge", {"-begin", "-end"}}};
    if (context.level == 0) {
      if (merging && name != "iCall" && name != "iNote" &&
          !(name == "iMerge" && literal(1) == "-end")) {
        Error(words[0].range,
              "only iCall and iNote are permitted inside iMerge");
      }
      if (name == "iMerge" && words.size() == 2) {
        if (literal(1) == "-begin") {
          merging = true;
          merge_range = words[0].range;
        } else if (literal(1) == "-end") {
          if (!merging) {
            Error(words[0].range, "iMerge -end has no matching -begin");
          }
          merging = false;
        }
      }
    }
    const auto option = kOptions.find(name);
    if (option != kOptions.end() && words.size() == 2 &&
        (words[1].literal || context.level == 0) &&
        !option->second.count(literal(1))) {
      Error(words[1].range, "invalid option for " + name + ": " + literal(1));
    }
    if (name == "iTake") {
      size_t start = 1;
      if (words.size() > 1 &&
          (!words[1].literal || literal(1).empty() || literal(1)[0] != '-')) {
        start = 2;
      }
      if (words.size() > start &&
          (words.size() != start + 1 ||
           (words[start].literal && literal(start) != "-persistent"))) {
        Error(words[start].range,
              "iTake expects a resource and/or -persistent");
      }
    }
    const auto check_options = [&](size_t start,
                                   const std::map<std::string, int>& allowed) {
      std::set<std::string> seen;
      for (size_t i = start; i < words.size(); ++i) {
        if (!words[i].literal && context.level != 0) return;
        const auto found = allowed.find(literal(i));
        if (found == allowed.end() || !seen.insert(literal(i)).second ||
            i + found->second >= words.size()) {
          Error(words[i].range, "invalid or incomplete option for " + name);
          return;
        }
        i += found->second;
      }
    };
    const auto check_number = [&](size_t index, bool enumeration, bool scan) {
      if (index >= words.size() ||
          (!words[index].literal && context.level != 0)) {
        return;
      }
      std::string value = literal(index);
      if (scan) {
        std::string digits;
        digits.reserve(value.size());
        for (size_t j = 0; j < value.size(); ++j) {
          const char c = value[j];
          if (c == '\\' && j + 1 < value.size() &&
              (value[j + 1] == '\n' ||
               (value[j + 1] == '\r' && j + 2 < value.size() &&
                value[j + 2] == '\n'))) {
            continue;
          }
          if (c != ' ' && c != '\t' && c != '\r' && c != '\n') digits += c;
        }
        // Scan whitespace may separate digits, never the prefix characters.
        const auto prefix = value.find_first_not_of(" \t\r\n");
        if (digits != value &&
            (prefix == std::string::npos || value.size() - prefix < 2 ||
             value[prefix] != '0' ||
             (value[prefix + 1] != 'x' && value[prefix + 1] != 'X' &&
              value[prefix + 1] != 'b' && value[prefix + 1] != 'B'))) {
          Error(words[index].range,
                "multiline iScan data requires a binary or hex prefix");
        }
        value = std::move(digits);
      }
      if (!IsNumber(value) && !(enumeration && IsScalar(value))) {
        Error(words[index].range, "invalid PDL value for " + name);
      }
    };
    if (name == "iWrite" || name == "iRead" || name == "iState") {
      check_number(2, name != "iState", false);
    }
    if (name == "iRunLoop" && words.size() > 1 &&
        (words[1].literal || context.level == 0) && literal(1) != "-time") {
      check_number(literal(1) == "-tck" ? 2 : 1, false, false);
    }
    if (name == "iWrite") {
      check_options(3, {{"-ignore_when_value_already_present", 1}});
      if (words.size() == 5 && words[4].literal && literal(4) != "auto" &&
          literal(4) != "off" && literal(4) != "on") {
        Error(words[4].range, "iWrite ignore option expects auto, off or on");
      }
    }
    if (name == "iClockOverride") {
      check_options(
          2, {{"-source", 1}, {"-freqMultiplier", 1}, {"-freqDivider", 1}});
      for (size_t i = 2; i + 1 < words.size(); i += 2) {
        if (literal(i) == "-source") {
          check_identifier(i + 1, IsHierSignal);
        } else if (literal(i) == "-freqMultiplier" ||
                   literal(i) == "-freqDivider") {
          check_number(i + 1, false, false);
        }
      }
    }
    if (name == "iGetReadData" || name == "iGetMiscompares") {
      check_options(2, {{"-chain", 1}, {"-hex", 0}, {"-bin", 0}, {"-dec", 0}});
      bool had_format = false;
      for (size_t i = 2; i < words.size() && words[i].literal; ++i) {
        if (literal(i) == "-chain") {
          check_identifier(++i, IsScalar, true);
        } else if (literal(i) == "-hex" || literal(i) == "-bin" ||
                   literal(i) == "-dec") {
          if (had_format) {
            Error(words[i].range, "readback formats are mutually exclusive");
          }
          had_format = true;
        }
      }
    }
    if (name == "iState") {
      check_options(3, {{"-LastWrittenValue", 0},
                        {"-LastReadValue", 0},
                        {"-LastMiscompareValue", 0}});
    }
    if (name == "iSetFail") check_options(2, {{"-quit", 0}});
    if (name == "iRunLoop" && words.size() >= 2 &&
        (words[1].literal || context.level == 0)) {
      if (literal(1) == "-time") {
        if (words.size() != 3) {
          Error(words[1].range, "iRunLoop -time requires a time value");
        } else if ((words[2].literal || context.level == 0) &&
                   !IsTime(literal(2))) {
          Error(words[2].range, "invalid PDL time value");
        }
      } else if (literal(1) == "-tck") {
        if (words.size() != 3) {
          Error(words[1].range, "iRunLoop -tck requires a cycle count");
        }
      } else if (words.size() > 2 && (words[2].literal || context.level == 0)) {
        if ((literal(2) != "-tck" || words.size() != 3) &&
            (literal(2) != "-sck" || words.size() != 4)) {
          Error(words[2].range, "invalid iRunLoop clock option");
        }
        if (literal(2) == "-sck") check_identifier(3, IsHierSignal);
      }
    }
    if (name == "iScan" && words.size() >= 3 &&
        (words[1].literal || context.level == 0)) {
      const size_t interface = literal(1) == "-ir" ? 2 : 1;
      check_identifier(interface, IsInstancePath, true);
      size_t i = interface + 1;
      if (literal(i) == "-chain") {
        check_identifier(i + 1, IsScalar, true);
        i += 2;
      }
      if (i >= words.size()) {
        Error(words[0].range, "iScan requires a scan length");
      } else if (words[i].literal || context.level == 0) {
        check_number(i, false, false);
        check_options(i + 1, {{"-si", 1}, {"-so", 1}, {"-stable", 0}});
        for (size_t j = i + 1; j + 1 < words.size(); ++j) {
          if (literal(j) == "-si" || literal(j) == "-so") {
            check_number(++j, false, true);
          }
        }
      }
    }
    if (name == "iOverrideScanInterface") {
      size_t i = 1;
      while (i < words.size() && (literal(i).empty() || literal(i)[0] != '-')) {
        ++i;
      }
      if (i == 1) {
        Error(words[0].range, "iOverrideScanInterface requires an interface");
      }
      for (size_t j = 1; j < i; ++j) {
        check_identifier(j, IsInstancePath, true);
      }
      check_options(i, {{"-capture", 1}, {"-update", 1}, {"-broadcast", 1}});
      for (size_t j = i + 1; j < words.size(); j += 2) {
        if ((words[j].literal || context.level == 0) && literal(j) != "on" &&
            literal(j) != "off") {
          Error(words[j].range, "scan interface options require on or off");
        }
      }
    }
    Context child = context;
    if ((name == "iProc" || name == "proc") && words.size() == 4 &&
        words[1].literal) {
      if (name == "iProc" && !context.has_binding) {
        Error(words[0].range, "iProc requires a preceding iProcsForModule");
      }
      child.owner = static_cast<int>(declarations_.size());
      child.prefix.clear();
      child.prefix_known = true;
      if (name == "iProc") {
        child.call_namespace.clear();
        child.call_namespace_known = true;
      }
      auto qualified =
          SplitQualified(Qualify(context.tcl_namespace, literal(1)));
      if (name == "proc") {
        child.tcl_namespace = qualified.first;
        child.tcl_namespace_known =
            context.tcl_namespace_known || literal(1).rfind("::", 0) == 0;
      }
      Declare({name == "proc" ? qualified.second : literal(1), name,
               name + " " + literal(1) + " " + Text(source_, words[2].range),
               name == "iProc" ? context.binding : "",
               name == "iProc" ? context.name_space : qualified.first,
               words[1].content, command.range, -1, "",
               name == "iProc" ? context.namespace_known
                               : child.tcl_namespace_known});
      bool valid = true;
      const auto arguments = words[2].literal
                                 ? SplitList(words[2].value, &valid)
                                 : std::vector<std::string>{};
      if (!valid) Error(words[2].range, "invalid procedure argument list");
      bool had_default = false;
      const auto argument_spans = ListSpans(source_, words[2].content);
      const auto content = [&](ByteRange range) {
        if (range.end > range.begin &&
            (source_[range.begin] == '{' || source_[range.begin] == '"')) {
          ++range.begin;
          --range.end;
        }
        return range;
      };
      std::set<std::string> names;
      for (size_t i = 0; i < arguments.size(); ++i) {
        const auto parts = SplitList(arguments[i], &valid);
        if (!valid || parts.empty() || parts.size() > 2) {
          Error(words[2].range, "invalid procedure argument");
          continue;
        }
        if (name == "iProc" &&
            (!IsScalar(parts[0]) ||
             (context.level == 0 && parts.size() == 2 && !IsNumber(parts[1]) &&
              !IsHierSignal(parts[1])))) {
          Error(words[2].range, "invalid PDL procedure argument or default");
        }
        if (name == "iProc" && had_default && parts.size() == 1) {
          Error(words[2].range,
                "required iProc argument follows a default argument");
        }
        had_default |= parts.size() == 2;
        if (!names.insert(parts[0]).second) {
          if (name == "iProc") {
            Error(words[2].range, "duplicate procedure argument");
          }
          continue;
        }
        if (i >= argument_spans.size()) continue;
        const auto fields = ListSpans(source_, content(argument_spans[i]));
        if (fields.empty()) continue;
        const auto name_range = content(fields.front());
        // Decoded names need a source map; never guess another occurrence.
        if (Text(source_, name_range) != parts[0]) continue;
        Declare({parts[0], "Variable", "argument " + parts[0], "", "",
                 name_range, name_range, child.owner});
      }
    } else if (name == "namespace" && literal(1) == "eval") {
      child.tcl_namespace = Qualify(context.tcl_namespace, literal(2));
      child.tcl_namespace_known =
          words.size() > 2 && words[2].literal &&
          (context.tcl_namespace_known || literal(2).rfind("::", 0) == 0);
    }
    if (name == "global" && context.owner >= 0) {
      for (size_t i = 1; i < words.size(); ++i) {
        if (!words[i].literal) continue;
        const auto qualified = SplitQualified(Qualify("", literal(i)));
        variable_bindings_[{context.owner, "", qualified.second}] = {
            -1, qualified.first, qualified.second};
        uses_.push_back({words[i].content, qualified.second, "Variable", "",
                         qualified.first, -1, -1});
      }
    }
    if (name == "set" || name == "variable") {
      const size_t end =
          name == "set" ? std::min(words.size(), size_t{2}) : words.size();
      for (size_t i = 1; i < end; i += 2) {
        const auto raw = Text(source_, words[i].content);
        const auto variable =
            ArrayName(words[i].literal ? words[i].value : raw);
        // A dynamic index still names a static array; a dynamic base does not.
        if (!words[i].literal &&
            (name != "set" || variable == raw ||
             variable.find_first_of("$[\\\"{ \t\r\n") != std::string::npos)) {
          continue;
        }
        const bool global = name == "variable" || context.owner < 0 ||
                            variable.find("::") != std::string::npos;
        const auto qualified =
            global ? SplitQualified(Qualify(context.tcl_namespace, variable))
                   : std::make_pair(std::string(), variable);
        VariableKey key{global ? -1 : context.owner, qualified.first,
                        qualified.second};
        const bool known = !global || context.tcl_namespace_known ||
                           variable.rfind("::", 0) == 0;
        // Keep an unknown namespace binding distinct from a procedure local.
        if (!known) std::get<0>(key) = -2;
        if (name == "variable" && context.owner >= 0) {
          variable_bindings_[{context.owner, "", qualified.second}] = key;
        }
        const auto binding = variable_bindings_.find(key);
        if (binding != variable_bindings_.end()) key = binding->second;
        if (std::get<0>(key) == -2) continue;
        auto range = words[i].content;
        range.end = range.begin + ArrayName(raw).size();
        if (name == "set" && words.size() == 2) {
          uses_.push_back({range, std::get<2>(key), "Variable", "",
                           std::get<1>(key), std::get<0>(key), -1});
        } else {
          Declare({std::get<2>(key), "Variable", name + " " + variable, "",
                   std::get<1>(key), range, words[i].range, std::get<0>(key)});
        }
      }
    }
    if (name == "iCall" && words.size() >= 2 && words[1].literal) {
      uses_.push_back({words[1].content, literal(1), "iProc", context.binding,
                       context.call_namespace, context.owner, -1,
                       context.prefix,
                       context.call_namespace_known && context.prefix_known});
    } else if (spec == Specs().end() && words[0].literal) {
      uses_.push_back(
          {words[0].content, name, "proc", "", context.tcl_namespace,
           context.owner, -1, "",
           context.tcl_namespace_known || name.rfind("::", 0) == 0});
    }
    static const std::set<std::string> kObjectCommands = {
        "iRead",           "iWrite", "iState",         "iTake",
        "iRelease",        "iClock", "iClockOverride", "iGetReadData",
        "iGetMiscompares", "iPrefix"};
    if (kObjectCommands.count(name) && words.size() > 1 &&
        !(name == "iPrefix" && literal(1) == "-")) {
      if ((words[1].literal || context.level == 0) &&
          !(name == "iTake" && literal(1) == "-persistent") &&
          !IsHierSignal(literal(1))) {
        Error(words[1].range, "invalid PDL object identifier");
      }
      std::string object = literal(1);
      if (!words[1].literal) {
        const auto raw = Text(source_, words[1].content);
        const auto bracket = raw.find('[');
        if (bracket != std::string::npos && raw.back() == ']' &&
            IsVector(raw.substr(bracket + 1, raw.size() - bracket - 2))) {
          const auto base = raw.substr(0, bracket);
          size_t start = 0;
          bool valid = true;
          do {
            const auto dot = base.find('.', start);
            valid &= IsScalar(base.substr(
                start, dot == std::string::npos ? dot : dot - start));
            if (dot == std::string::npos) break;
            start = dot + 1;
          } while (start <= base.size());
          if (valid) object = raw;
        }
      }
      if (!object.empty()) {
        uses_.push_back({words[1].content, object, "IclObject", context.binding,
                         "", context.owner, -1,
                         name == "iPrefix" ? "" : context.prefix,
                         name == "iPrefix" || context.prefix_known});
      }
    }
    contexts_.push_back({{command.range.begin, scope.end}, context});
    // Tcl reports a zero-length body token for an unfinished procedure.
    if (child.owner != context.owner && words.size() == 4) {
      auto body = words.back().content;
      if (words.back().range.begin == words.back().range.end) {
        body.end = command.range.end;
      }
      contexts_.push_back({body, child});
    }
    AnalyzeWords(&words, context, child);
  }
  if (merging) Error(merge_range, "iMerge -begin has no matching -end");
}

void Document::AnalyzeWords(std::vector<Word>* words, const Context& context,
                            const Context& child) {
  for (auto& word : *words) {
    for (const auto& variable : word.variables) {
      const auto name = ArrayName(variable.second);
      const bool global =
          context.owner < 0 || name.find("::") != std::string::npos;
      const auto qualified =
          global ? SplitQualified(Qualify(context.tcl_namespace, name))
                 : std::make_pair(std::string(), name);
      auto range = variable.first;
      range.end = range.begin + name.size();
      uses_.push_back(
          {range, qualified.second, "Variable", "", qualified.first,
           global ? -1 : context.owner, -1, "",
           !global || context.tcl_namespace_known || name.rfind("::", 0) == 0});
    }
    if (!word.substitutions.empty()) {
      Analyze(&word.substitutions, context, word.content);
    }
    if (word.body || !word.script.empty()) {
      Analyze(&word.script, child, word.content);
    }
  }
}

const Document::Context& Document::ContextAt(size_t byte) const {
  for (auto it = contexts_.rbegin(); it != contexts_.rend(); ++it) {
    if (it->range.begin <= byte && byte <= it->range.end) return it->context;
  }
  static const Context kEmpty;
  return kEmpty;
}

const NameUse* Document::UseAt(size_t byte) const {
  for (const auto& use : uses_) {
    if (use.range.begin <= byte && byte < use.range.end) return &use;
  }
  return nullptr;
}

std::string Document::Format(int indent_width) const {
  if (!ok()) return "";
  std::string output;
  Render(source_, commands_, 0, ClampIndentWidth(indent_width), &output);
  return output;
}

std::string Document::Dump() const {
  std::ostringstream out;
  std::function<void(const std::vector<Command>&, int)> visit =
      [&](const auto& commands, int depth) {
        for (const auto& command : commands) {
          if (!command.words.empty()) {
            out << std::string(depth * 2, ' ') << command.words[0].value
                << '\n';
          }
          for (const auto& word : command.words) {
            visit(word.script, depth + 1);
            visit(word.substitutions, depth + 1);
          }
        }
      };
  visit(commands_, 0);
  return out.str();
}

void Workspace::Reindex() {
  procedures_.clear();
  modules_.clear();
  members_.clear();
  uris_.clear();
  for (const auto& item : documents_) {
    const auto* doc = item.second.get();
    uris_[doc] = item.first;
    std::map<std::string, size_t> last;
    for (size_t i = 0; i < doc->declarations().size(); ++i) {
      const auto& d = doc->declarations()[i];
      if (!doc->CanFormat() && d.module >= 0 && d.kind != "Module") {
        members_[doc][{d.module, d.name}].push_back({doc, i});
      }
      if (d.kind == "Module") {
        const auto key =
            d.name_space.empty() ? d.name : d.name_space + "::" + d.name;
        modules_[key].push_back({doc, i});
      }
      if (d.kind == "iProc" || d.kind == "proc") {
        // Runtime binding or namespace values have no static target.
        if (!d.static_context || (d.kind == "iProc" && d.binding.empty())) {
          continue;
        }
        last[ProcedureKey(d.binding, d.name_space, d.name)] = i;
      }
    }
    for (const auto& entry : last) {
      procedures_[entry.first].push_back({doc, entry.second});
    }
  }
}

SymbolRef Workspace::IclModule(const std::string& name) const {
  const auto found = modules_.find(name);
  return found != modules_.end() && found->second.size() == 1
             ? found->second.front()
             : SymbolRef{};
}

SymbolRef Workspace::IclObject(const std::string& binding,
                               const std::string& path) const {
  auto scope = IclModule(binding);
  size_t begin = 0;
  while (scope && begin < path.size()) {
    const size_t dot = path.find('.', begin);
    const auto end = dot == std::string::npos ? path.size() : dot;
    std::string name = path.substr(begin, end - begin);
    name.erase(name.find_first_of("([") == std::string::npos
                   ? name.size()
                   : name.find_first_of("(["));
    const auto document = members_.find(scope.document);
    if (document == members_.end()) return {};
    const auto found =
        document->second.find({static_cast<int>(scope.index), name});
    if (found == document->second.end() || found->second.size() != 1) return {};
    const auto member = found->second.front();
    if (dot == std::string::npos) return member;
    if (member.declaration().kind != "Instance") return {};
    scope = IclModule(member.declaration().target);
    begin = dot + 1;
  }
  return scope;
}

SymbolRef Workspace::Resolve(const Document& document,
                             const NameUse& use) const {
  if (use.declaration >= 0) {
    return {&document, static_cast<size_t>(use.declaration)};
  }
  if (!use.static_context) return {};
  if (use.kind == "IclModule") return IclModule(use.name);
  if (use.kind == "iProc" && use.binding.empty()) return {};
  const auto path = use.prefix.empty() ? use.name : use.prefix + "." + use.name;
  if (use.kind == "IclObject") return IclObject(use.binding, path);
  if (use.kind == "Variable") {
    Document::VariableKey key{use.module, use.name_space, use.name};
    const auto binding = document.variable_bindings_.find(key);
    if (binding != document.variable_bindings_.end()) key = binding->second;
    const auto found = document.variables_.find(key);
    return found == document.variables_.end()
               ? SymbolRef{}
               : SymbolRef{&document, static_cast<size_t>(found->second)};
  }
  if (use.kind == "proc") {
    const auto qualified = SplitQualified(Qualify(use.name_space, use.name));
    auto found =
        procedures_.find(ProcedureKey("", qualified.first, qualified.second));
    if (found == procedures_.end() && use.name.rfind("::", 0) != 0) {
      const auto global = SplitQualified(use.name);
      found = procedures_.find(ProcedureKey("", global.first, global.second));
    }
    return found != procedures_.end() && found->second.size() == 1
               ? found->second.front()
               : SymbolRef{};
  }
  std::string ns = use.name_space, name = path, binding = use.binding;
  const size_t dot = name.rfind('.');
  if (dot != std::string::npos) {
    const auto instance = IclObject(binding, name.substr(0, dot));
    if (!instance || instance.declaration().kind != "Instance") return {};
    binding = instance.declaration().target;
    name.erase(0, dot + 1);
  }
  const size_t separator = name.rfind("::");
  if (separator != std::string::npos) {
    ns = name.substr(0, separator);
    name.erase(0, separator + 2);
  }
  const auto found = procedures_.find(ProcedureKey(binding, ns, name));
  if (found == procedures_.end() || found->second.size() != 1) return {};
  return found->second.front();
}

std::vector<SymbolRef> Workspace::Complete(const Document& document,
                                           size_t byte,
                                           bool* member_context) const {
  *member_context = false;
  const auto& context = document.ContextAt(byte);
  const int owner = context.owner;
  std::vector<SymbolRef> result;
  for (size_t i = 0; i < document.declarations().size(); ++i) {
    const auto& d = document.declarations()[i];
    if (d.kind == "Variable" && d.module == owner &&
        (owner >= 0 || context.tcl_namespace_known) &&
        d.name_space == (owner < 0 ? context.tcl_namespace : "")) {
      result.push_back({&document, i});
    }
  }
  for (const auto& alias : document.variable_bindings_) {
    if (std::get<0>(alias.first) != owner) continue;
    const auto found = document.variables_.find(alias.second);
    if (found != document.variables_.end()) {
      result.push_back({&document, static_cast<size_t>(found->second)});
    }
  }
  for (const auto& item : procedures_) {
    if (item.second.size() != 1) continue;
    const auto symbol = item.second.front();
    const auto& d = symbol.declaration();
    const bool visible =
        d.kind == "proc"
            ? (context.tcl_namespace_known &&
               (d.name_space == context.tcl_namespace || d.name_space.empty()))
            : (context.has_binding && !context.binding.empty() &&
               context.call_namespace_known && d.binding == context.binding &&
               d.name_space == context.call_namespace);
    if (visible) {
      result.push_back(symbol);
    }
  }
  return result;
}

std::string Workspace::Uri(const Document* document) const {
  const auto found = uris_.find(document);
  return found == uris_.end() ? "" : found->second;
}

const std::vector<std::string>& Keywords() {
  static const std::vector<std::string> kKeywords = [] {
    std::vector<std::string> words;
    for (const auto& entry : Specs()) words.push_back(entry.first);
    for (const auto* word :
         {"proc", "set", "if", "elseif", "else", "foreach", "for", "while",
          "switch", "return", "expr", "catch", "try", "namespace", "variable",
          "incr", "list", "lindex", "dict"}) {
      words.emplace_back(word);
    }
    return words;
  }();
  return kKeywords;
}

std::string FormatOutcome::Diagnostic(const std::string& name) const {
  return name + ": line " + std::to_string(error_line) + ": " + error_message;
}

FormatOutcome FormatParsedDocument(const Document& document,
                                   const FormatRequest& request) {
  FormatOutcome result;
  if (!document.CanFormat()) {
    result.error_line = 1;
    result.error_message = "ICL documents cannot be formatted as PDL";
    return result;
  }
  result.ok = document.ok();
  if (!result.ok) {
    result.error_line = document.error_line();
    result.error_message = document.error_message();
  } else {
    result.text = document.Format(request.indent_width);
    if (request.with_ast_dump) result.ast_dump = document.Dump();
  }
  return result;
}

FormatOutcome FormatDocument(const std::string& source,
                             const FormatRequest& request) {
  return FormatParsedDocument(Document(source), request);
}

}  // namespace pdl
