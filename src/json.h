// Copyright 2024 The ICL Format Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// A JSON value, written for one purpose: speaking LSP over stdio.

#ifndef ICL_FORMAT_SRC_JSON_H_
#define ICL_FORMAT_SRC_JSON_H_

#include <string>
#include <utility>
#include <vector>

namespace icl {

// ponytail: a hand-rolled value type instead of a JSON library. The whole
// point of this build is that it needs only a C++17 compiler plus bison/flex;
// a dependency would be permanent, while the subset LSP actually uses (parse a
// request, build a response) is ~200 lines. Upgrade path: if this ever needs
// JSON Pointer, schema validation or streaming, replace with a real library.
class Json {
 public:
  enum class Type { kNull, kBool, kNumber, kString, kArray, kObject };

  using Member = std::pair<std::string, Json>;
  using Members = std::vector<Member>;
  using Elements = std::vector<Json>;

  Json() = default;

  static Json Null() { return Json(); }
  static Json Bool(bool value);
  static Json Int(long long value);
  static Json Number(double value);
  static Json String(std::string value);
  static Json Array(Elements elements);
  static Json Object(Members members);

  Type type() const { return type_; }
  bool is_null() const { return type_ == Type::kNull; }

  // Object lookup. Nullptr when the key is absent or this is not an object,
  // so a malformed request degrades into "field missing" rather than crashing.
  const Json* Find(const std::string& key) const;

  // Typed accessors, each returning |fallback| when the value has another
  // type. LSP clients differ in optional fields; being lenient on read keeps
  // one code path instead of a type check per field.
  //
  // Only the types this server actually reads exist: int for ids and tabSize,
  // string for methods and uris, elements for contentChanges. Adding a
  // bool_value the day a bool field is read is cheaper than carrying one now.
  long long int_value(long long fallback = 0) const;
  std::string string_value(const std::string& fallback = "") const;
  const Elements& elements() const { return elements_; }

  std::string Serialize() const;

  // Parses one complete JSON document.
  static bool Parse(const std::string& text, Json* out, std::string* error);

 private:
  Type type_ = Type::kNull;
  bool bool_ = false;
  double number_ = 0.0;
  bool number_is_int_ = false;
  std::string string_;
  Elements elements_;
  Members members_;
};

}  // namespace icl

#endif  // ICL_FORMAT_SRC_JSON_H_
