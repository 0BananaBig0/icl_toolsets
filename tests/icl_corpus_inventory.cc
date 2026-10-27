// SPDX-License-Identifier: MIT
// Test support only: reuse the production frontend for lossless module boundaries.
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include "icl/icl_semantics.h"
#include "utility/json.h"

using icl::Json;

// Generic bodies deliberately bypass the ICL token stream. Retain their exact
// source, including whitespace, instead of treating it as insignificant trivia.
struct OpaqueRanges : icl::AstVisitor {
  explicit OpaqueRanges(const icl::ParsedDocument& doc) : doc(doc) {}
  const icl::ParsedDocument& doc;
  std::vector<icl::ByteRange> ranges;
  void Visit(const icl::AstComment&) override {}
  void Visit(const icl::AstStatement& node) override {
    if (node.text().compare(0, 11, "AccessLink ") == 0)
      ranges.push_back(doc.Range(node.span()));
  }
  void Visit(const icl::AstBlock& node) override {
    for (const auto* child : node.body()) child->Accept(*this);
  }
};

Json Inventory(const std::string& source, bool audit) {
  icl::ParsedDocument doc(source);
  if (!doc.ok()) {
    if (audit) return Json::Object({{"ok", Json::Bool(false)}});
    throw std::runtime_error("ICL parse failed");
  }
  OpaqueRanges opaque(doc);
  for (const auto* node : doc.driver().unit().top_level()) node->Accept(opaque);
  const auto& tokens = doc.driver().unit().tokens();
  Json::Elements modules, comments;
  for (size_t i = 0; i < doc.declarations().size(); ++i) {
    const auto& d = doc.declarations()[i];
    if (d.kind != "Module") continue;
    Json::Elements spelling, bodies;
    auto token = std::lower_bound(tokens.begin(), tokens.end(), d.range.begin,
        [](const icl::SourceToken& t, size_t begin) { return t.begin < begin; });
    for (; token != tokens.end() && token->begin < d.range.end; ++token)
      spelling.push_back(Json::String(token->text));
    for (auto r : opaque.ranges)
      if (r.begin >= d.range.begin && r.end <= d.range.end)
        bodies.push_back(Json::String(source.substr(r.begin, r.end - r.begin)));
    modules.push_back(Json::Object({
        {"name", Json::String(d.name)}, {"namespace", Json::String(d.name_space)},
        {"use_namespace", Json::String(doc.namespace_uses().at(i).begin()->second)},
        {"begin", Json::Int(d.range.begin)}, {"end", Json::Int(d.range.end)},
        {"tokens", Json::Array(std::move(spelling))},
        {"opaque", Json::Array(std::move(bodies))}}));
  }
  // The scanner omits comments/whitespace. Uncovered spans preserve all original
  // notices, including inter-token comments that are embedded in AST strings.
  auto covered = opaque.ranges;
  for (const auto& t : tokens) covered.push_back({t.begin, t.end});
  covered.push_back({source.size(), source.size()});
  std::sort(covered.begin(), covered.end(), [](auto a, auto b) { return a.begin < b.begin; });
  size_t end = 0;
  for (auto r : covered) {
    if (r.begin > end) {
      const auto gap = source.substr(end, r.begin - end);
      if (gap.find_first_not_of(" \r\n\t") != std::string::npos)
        comments.push_back(Json::Object({{"begin", Json::Int(end)},
            {"end", Json::Int(r.begin)}, {"text", Json::String(gap)}}));
    }
    end = std::max(end, r.end);
  }
  return Json::Object({{"ok", Json::Bool(true)},
                       {"modules", Json::Array(std::move(modules))},
                       {"comments", Json::Array(std::move(comments))}});
}

int main(int argc, char** argv) {
  Json::Elements files;
  const bool audit = argc > 1 && std::string(argv[1]) == "--audit";
  for (int i = audit ? 2 : 1; i < argc; ++i) {
    try {
      std::ifstream input(argv[i], std::ios::binary);
      if (!input) throw std::runtime_error("cannot open input");
      const std::string source{std::istreambuf_iterator<char>(input), {}};
      if (input.bad()) throw std::runtime_error("cannot read input");
      files.push_back(Inventory(source, audit));
    } catch (const std::exception& error) {
      std::cerr << argv[i] << ": " << error.what() << '\n';
      return 2;
    }
  }
  std::cout << Json::Array(std::move(files)).Serialize() << '\n';
}
