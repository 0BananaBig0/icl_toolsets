// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_DESIGN_H_
#define ICL_TOOLSETS_SRC_PATTERN_DESIGN_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "icl_semantics.h"
#include "pattern_bsdl.h"

namespace pattern {

struct ModuleDefinition {
  std::string path;
  std::shared_ptr<const icl::ParsedDocument> document;
  size_t declaration_index = 0;
  const icl::AstBlock* ast = nullptr;
  std::vector<size_t> instances;

  const icl::Declaration& declaration() const {
    return document->declarations().at(declaration_index);
  }
  std::string name() const;
  std::string Location(size_t byte) const;
};

// Runtime loading has ordered replacement semantics; editor workspaces do not.
class Design {
 public:
  void AddIcl(const std::string& path, const std::string& text);
  // BSDL has separate identities; duplicate entities are rejected, not
  // replaced.
  void AddBsdl(const std::string& path, const std::string& text,
               const std::string& entity);
  const BsdlTap& ResolveBsdl(const std::string& entity) const;
  const ModuleDefinition& SelectTop(const std::string& name);
  const ModuleDefinition& ResolveInstance(const ModuleDefinition& owner,
                                          size_t declaration_index) const;
  const std::map<std::string, ModuleDefinition>& modules() const {
    return modules_;
  }
  const std::vector<std::string>& warnings() const { return warnings_; }

 private:
  void CheckHierarchy(const ModuleDefinition& top) const;
  std::map<std::string, ModuleDefinition> modules_;
  std::map<std::string, BsdlTap> bsdl_taps_;
  std::vector<std::string> warnings_;
};

}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_DESIGN_H_
