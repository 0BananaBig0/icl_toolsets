// Copyright 2026 The ICL Toolsets Authors.
// SPDX-License-Identifier: MIT
#ifndef ICL_TOOLSETS_SRC_PATTERN_PATTERN_ADDRESS_H_
#define ICL_TOOLSETS_SRC_PATTERN_PATTERN_ADDRESS_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "pattern/pattern_netlist.h"

namespace pattern {
enum class AddressOperation { kWrite, kRead };
struct AddressRequirement {
  std::shared_ptr<const std::vector<BitReference>> signals;
  // Unsized, significant bits; never truncates to the address bus width.
  Constant value;
  BitReference enable;
};

// The netlist must outlive the resolver and remain immutable. Resolution checks
// address/control connectivity; it neither changes hardware state nor
// schedules.
class AddressResolver {
 public:
  explicit AddressResolver(const Netlist& netlist);
  AddressRequirement Resolve(const std::string& target,
                             AddressOperation operation);
  // Resolve explicit bindings and inherited addressed-instance control ports.
  BitReference ResolveEnable(size_t scope, AddressOperation operation) const;

 private:
  struct Ports {
    std::vector<size_t> address;
    size_t address_width = 0;
    std::vector<size_t> write;
    std::vector<size_t> read;
  };
  struct Domain {
    std::shared_ptr<const std::vector<BitReference>> signals;
    std::string offset;
    std::optional<size_t> scan_register;
  };
  const Domain& ResolveDomain(size_t scope);
  BitReference Terminal(BitReference bit, size_t stop_scope,
                        const std::string& stop_kind) const;
  bool ParentAddress(const std::vector<BitReference>& bits, size_t scope) const;
  std::vector<BitReference> AddressBits(size_t scope) const;

  const Netlist& netlist_;
  std::vector<Ports> ports_;
  std::vector<std::optional<Domain>> domains_;
  size_t retained_bits_ = 0;
};
}  // namespace pattern
#endif  // ICL_TOOLSETS_SRC_PATTERN_PATTERN_ADDRESS_H_
