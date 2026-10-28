# Copyright 2024 The ICL Format Authors.
#
# SPDX-License-Identifier: MIT
# See LICENSE in the project root for license information.
#
# Builds five programs for IEEE 1687-2014 (IJTAG) ICL and PDL:
#   icl-format  command line formatter
#   icl-lsp     Language Server (diagnostics + formatting) for editors
#   pdl-format  Tcl-aware PDL command line formatter
#   pdl-lsp     PDL language services for editors
#   icl-pattern-generation  experimental serial PDL retargeting
# Each is produced dynamically linked and fully static: ten binaries.
#
# Pipeline:  text --(flex/bison)--> AST --(layout)--> lines --(render)--> text
# Each language pair shares its own syntax tree, analysis and formatter.
#
# Targets:
#   make            build all ten binaries
#   make dynamic    build all five dynamic binaries
#   make install    install into $(prefix) (default: $HOME/.local)
#   make uninstall  remove exactly what install put there
#   make check      run the test suite
#   make clean      remove build output (keeps bison/flex output)
#   make distclean  remove everything generated
#
# Requirements: bison >= 3.8, flex >= 2.6, GNU make >= 4.3, C++17, Tcl development files.

CXX ?= g++
BISON ?= bison
FLEX ?= flex
CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy

PACKAGE := icl-format
VERSION := 1.0.0

# ---- installation layout (GNU style, rooted at $HOME/.local by default) ----
prefix ?= $(HOME)/.local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
libdir ?= $(exec_prefix)/lib
includedir ?= $(prefix)/include
datarootdir ?= $(prefix)/share
docdir ?= $(datarootdir)/doc/$(PACKAGE)
mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1

INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 0755
INSTALL_DATA ?= $(INSTALL) -m 0644

# ---- build layout ----
SRC_DIR := src
BUILD_DIR := build

# When `.prebuilt` is present the generated parser/scanner are shipped in ./gen
# and bison/flex are not needed at all (see the `dist` target). This is how a
# stock Rocky Linux 8 box, whose bison is too old, can still build the tool.
PREBUILT := $(shell test -f .prebuilt && echo 1 || echo 0)
ifeq ($(PREBUILT),1)
GEN_DIR := gen
else
GEN_DIR := $(BUILD_DIR)/gen
endif

# Output of `make codegraph`. Outside the PREBUILT conditional above on
# purpose: it does not depend on how the parser was built, and putting it
# inside meant it was empty in the normal build, so `mkdir -p $(CODEGRAPH_DIR)`
# expanded to a bare `mkdir` and failed.
CODEGRAPH_DIR := .codegraph

# ICL programs share their front end; PDL programs share a separate one:
#   icl-format  the command line formatter
#   icl-lsp     a Language Server (diagnostics + formatting) for editors
# Each is produced both dynamically linked and fully static.
BIN := $(BUILD_DIR)/$(PACKAGE)
LSP_BIN := $(BUILD_DIR)/icl-lsp
STATIC_BIN := $(BUILD_DIR)/$(PACKAGE)-static
LSP_STATIC_BIN := $(BUILD_DIR)/icl-lsp-static
PDL_BIN := $(BUILD_DIR)/pdl-format
PDL_LSP_BIN := $(BUILD_DIR)/pdl-lsp
PDL_STATIC_BIN := $(BUILD_DIR)/pdl-format-static
PDL_LSP_STATIC_BIN := $(BUILD_DIR)/pdl-lsp-static

# Tcl stub archives cannot supply the parser. Static builds need real libtcl.a.
TCL_CFLAGS ?= $(shell pkg-config --cflags tcl 2>/dev/null)
TCL_LIBS ?= $(shell pkg-config --libs tcl 2>/dev/null)
TCL_STATIC_LIB ?= $(firstword $(wildcard $(BUILD_DIR)/deps/tcl-static/libtcl9.0.a /usr/lib64/libtcl[89]*.a /usr/lib/libtcl[89]*.a $(shell pkg-config --variable=libdir tcl 2>/dev/null)/libtcl[89]*.a))
TCL_STATIC_LIBS ?= $(TCL_STATIC_LIB) -ldl -lpthread -lm

CXXFLAGS ?= -O2
CXXSTD := -std=c++17
WARNINGS := -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -I$(SRC_DIR) -I$(GEN_DIR) \
    -D$(shell echo $(PACKAGE) | tr a-z- A-Z_)_VERSION='"$(VERSION)"'

# Hand-written sources.
#
# The parser front end is shared: icl-lsp links the same objects as
# icl-format, so a grammar fix reaches both and neither can drift.
#
# format_job.cc is the shared *job* layer: everything between "an indent width
# was requested" and "here is the formatted text". Both binaries link it, so
# the two front ends cannot disagree about style clamping or error phrasing.
SHARED_SOURCES := \
    $(SRC_DIR)/icl/icl_driver.cc \
    $(SRC_DIR)/icl/format_tree.cc \
    $(SRC_DIR)/icl/format_job.cc \
    $(SRC_DIR)/icl/icl_semantics.cc

CLI_SOURCES := $(SRC_DIR)/icl/icl_format_main.cc
LSP_SOURCES := $(SRC_DIR)/utility/json.cc $(SRC_DIR)/icl/icl_lsp_main.cc
PDL_SOURCES := $(SRC_DIR)/pdl/pdl_document.cc $(SRC_DIR)/pdl/pdl_format_main.cc \
    $(SRC_DIR)/pdl/pdl_lsp_main.cc

SOURCES := $(SHARED_SOURCES) $(CLI_SOURCES) $(LSP_SOURCES) $(PDL_SOURCES)
PDL_OBJECT := $(BUILD_DIR)/pdl/pdl_document.o
PATTERN_SOURCES := $(SRC_DIR)/pattern/pattern_design.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_runtime.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_expression.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_netlist.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_logic.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_flatten.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_address.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_bsdl.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_access_link.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_tap.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_tap_network.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_state.cc
PATTERN_SOURCES += $(SRC_DIR)/pattern/pattern_tap_execution.cc
PATTERN_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(PATTERN_SOURCES))

# The constraint backend is separate from parser/editor and replay objects.
Z3_VERSION := 4.13.3
Z3_PREFIX ?= $(abspath $(BUILD_DIR)/deps/z3-$(Z3_VERSION))
Z3_CFLAGS ?= $(if $(wildcard $(Z3_PREFIX)/include/z3++.h),-isystem $(Z3_PREFIX)/include,$(shell pkg-config --cflags z3 2>/dev/null))
Z3_STATIC_LIB ?= $(firstword $(wildcard $(Z3_PREFIX)/lib/libz3.a $(Z3_PREFIX)/lib64/libz3.a /usr/lib/libz3.a /usr/lib64/libz3.a /usr/lib/x86_64-linux-gnu/libz3.a))
Z3_LIBS ?= $(if $(Z3_STATIC_LIB),$(Z3_STATIC_LIB) -lpthread,$(shell pkg-config --libs z3 2>/dev/null))
SOLVER_SOURCES := $(SRC_DIR)/pattern/pattern_symbolic.cc $(SRC_DIR)/pattern/pattern_values.cc $(SRC_DIR)/pattern/pattern_scheduler.cc
SOLVER_SOURCES += $(SRC_DIR)/pattern/pattern_merge.cc
SOLVER_SOURCES += $(SRC_DIR)/pattern/pattern_resources.cc
SOLVER_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SOLVER_SOURCES))
SOURCES += $(SOLVER_SOURCES)
$(SOLVER_OBJECTS): CPPFLAGS += $(Z3_CFLAGS)
$(SOLVER_OBJECTS): | check-z3

SOURCES += $(PATTERN_SOURCES) $(SRC_DIR)/pattern/icl_pattern_generation_main.cc

# bison / flex outputs. Kept in $(GEN_DIR) so that `make clean` cannot leave a
# stale generated parser behind a hand-edited .yy file.
PARSER_CC := $(GEN_DIR)/icl_parser.cc
PARSER_HH := $(GEN_DIR)/icl_parser.hh
LEXER_CC := $(GEN_DIR)/icl_lexer.cc
LEXER_HH := $(GEN_DIR)/icl_lexer.hh

GEN_SOURCES := $(PARSER_CC) $(LEXER_CC)

SHARED_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SHARED_SOURCES)) \
    $(patsubst $(GEN_DIR)/%.cc,$(BUILD_DIR)/%.o,$(GEN_SOURCES))
CLI_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(CLI_SOURCES))
LSP_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(LSP_SOURCES))

OBJECTS := $(SHARED_OBJECTS) $(CLI_OBJECTS) $(LSP_OBJECTS)
OBJECTS += $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(PDL_SOURCES))
OBJECTS += $(PATTERN_OBJECTS) $(SOLVER_OBJECTS) $(BUILD_DIR)/pattern/icl_pattern_generation_main.o

# ---- statically linked binary ----
#
# `-static` includes C/C++ runtimes and, for PDL, Tcl in the executable.
# Static Tcl still contains optional glibc NSS calls; see docs/pdl-tools.md
# for link warnings and portability limits. Test the intended target host.
.PHONY: all dynamic static install install-dynamic install-static uninstall \
    check nvimcheck coccheck clean distclean version dist check-tools \
    formatcheck namingcheck syntaxcheck keywordcheck pdl-dynamic pdl-static \
    icl-dynamic icl-static pdlcheck check-tcl check-tcl-static
.DEFAULT_GOAL := all

# Development-only style checks; normal builds need no LLVM tools. Review the
# embedded C++ in .l/.yy separately: neither file is a C++ translation unit.
formatcheck:
	$(CLANG_FORMAT) --dry-run --Werror $(SOURCES) $(wildcard $(SRC_DIR)/*/*.h)

namingcheck: $(GEN_SOURCES)
	$(CLANG_TIDY) $(SOURCES) -- $(CXXSTD) $(CPPFLAGS) $(TCL_CFLAGS) $(Z3_CFLAGS)

# ---- toolchain requirements ----
#
# The grammar uses Bison's C++ variant API (`api.value.type variant` +
# `api.token.constructor`), available since 3.0, but it is only *tested*
# against 3.8. Rocky Linux 8 ships bison 3.0.4, which is below that, so the
# build stops with an explanation instead of failing deep inside bison.
#
# Two ways forward on Rocky 8:
#   * install a newer bison, or
#   * build from a `make dist` tarball, which bundles the generated parser so
#     that only g++ and Tcl development files are needed.
BISON_MAJOR := $(shell $(BISON) --version 2>/dev/null | head -1 | \
    sed -E 's/.* ([0-9]+)\.([0-9]+).*/\1/')
BISON_MINOR := $(shell $(BISON) --version 2>/dev/null | head -1 | \
    sed -E 's/.* ([0-9]+)\.([0-9]+).*/\2/')

check-tools:
ifeq ($(PREBUILT),1)
	@echo "toolchain: prebuilt parser/scanner in gen/ — bison/flex not required"
else
	@if [ -z "$(BISON_MAJOR)" ]; then \
	  echo "error: bison not found. Install it (dnf install bison) or build" >&2; \
	  echo "       from a 'make dist' tarball, which needs only g++." >&2; \
	  exit 1; \
	fi
	@if [ "$(BISON_MAJOR)" -lt 3 ] || \
	   { [ "$(BISON_MAJOR)" -eq 3 ] && [ "$(BISON_MINOR)" -lt 8 ]; }; then \
	  echo "error: bison $(BISON_MAJOR).$(BISON_MINOR) is older than the required 3.8." >&2; \
	  echo "       Rocky Linux 8 ships 3.0.4. Either install a newer bison, or" >&2; \
	  echo "       build from a 'make dist' tarball (generated parser included)." >&2; \
	  exit 1; \
	fi
	@echo "toolchain: bison $(BISON_MAJOR).$(BISON_MINOR), $(CXX) $(shell $(CXX) --version | head -1)"
endif

all: dynamic static

# For a box without the static C runtime installed (glibc-static / libstdc++
# -static): build the four dynamic binaries only.
dynamic: icl-dynamic pdl-dynamic

icl-dynamic: check-tools $(BIN) $(LSP_BIN)

pdl-dynamic: check-tools check-tcl $(PDL_BIN) $(PDL_LSP_BIN)

check-tcl:
	@if [ -z "$(TCL_LIBS)" ]; then \
	  echo "error: install Tcl development files and pkg-config, or set TCL_CFLAGS/TCL_LIBS" >&2; \
	  exit 1; \
	fi

check-tcl-static:
	@if [ ! -f "$(TCL_STATIC_LIB)" ]; then \
	  echo "error: set TCL_STATIC_LIB to a real static Tcl archive; see docs/pdl-tools.md" >&2; \
	  exit 1; \
	fi

$(BUILD_DIR)/pdl/pdl_document.o: CPPFLAGS += $(TCL_CFLAGS)
$(BUILD_DIR)/pattern/pattern_runtime.o: CPPFLAGS += $(TCL_CFLAGS)

$(PDL_BIN): $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/pdl/pdl_format_main.o
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ $(TCL_LIBS) -o $@

$(PDL_LSP_BIN): $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/pdl/pdl_lsp_main.o $(BUILD_DIR)/utility/json.o
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ $(TCL_LIBS) -o $@

$(PDL_STATIC_BIN): $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/pdl/pdl_format_main.o | check-tcl-static
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ $(TCL_STATIC_LIBS) -o $@

$(PDL_LSP_STATIC_BIN): $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/pdl/pdl_lsp_main.o $(BUILD_DIR)/utility/json.o | check-tcl-static
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ $(TCL_STATIC_LIBS) -o $@

$(BIN): $(SHARED_OBJECTS) $(CLI_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

$(LSP_BIN): $(SHARED_OBJECTS) $(LSP_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

static: icl-static pdl-static

icl-static: check-tools $(STATIC_BIN) $(LSP_STATIC_BIN)

pdl-static: check-tools $(PDL_STATIC_BIN) $(PDL_LSP_STATIC_BIN)

pdlcheck: pdl-syntaxcheck
	python3 tests/src/test_pdl_tools.py --formatter $(PDL_BIN) --server $(PDL_LSP_BIN)
	python3 tests/tests/test_import_pdl_corpus.py
	python3 tests/src/test_pdl_corpus.py --formatter $(PDL_BIN) --server $(PDL_LSP_BIN)
	python3 tests/src/test_pdl_blocks.py --formatter $(PDL_BIN) --server $(PDL_LSP_BIN)

.PHONY: pdl-syntaxcheck
pdl-syntaxcheck: pdl-dynamic
	python3 tests/src/test_pdl_syntax_properties.py --formatter $(PDL_BIN) --server $(PDL_LSP_BIN)

.PHONY: vim-plugin-check vim-plugin-dist
vim-plugin-check:
	python3 tests/vim-icl-pdl-modern/test_plugin.py

.PHONY: goldencheck
goldencheck: $(BIN) $(PDL_BIN) $(LSP_BIN) $(PDL_LSP_BIN)
	python3 tests/tests/test_golden_test.py
	python3 tests/golden_test.py --binary $(BIN) --server $(LSP_BIN)
	python3 tests/golden_test.py --language pdl --binary $(PDL_BIN) --server $(PDL_LSP_BIN)

vim-plugin-dist:
	@mkdir -p $(BUILD_DIR)/dist
	tar -czf $(BUILD_DIR)/dist/vim-icl-pdl-modern.tar.gz vim-icl-pdl-modern

$(STATIC_BIN): $(SHARED_OBJECTS) $(CLI_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ -o $@

$(LSP_STATIC_BIN): $(SHARED_OBJECTS) $(LSP_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ -o $@

# -MMD -MP: write a .d file listing the headers each object depends on, and
# emit a phony target for every one of them. Without this, editing a header
# leaves stale objects in place — which is not a subtle bug: changing a
# private member of Json and running `make` produced a binary that segfaulted
# on its first message, because icl_lsp_main.o still used the old layout.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(PARSER_HH) | $(BUILD_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(GEN_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Generated dependencies. `-` so the first build, before any .d exists, is fine.
-include $(OBJECTS:.o=.d) $(GEN_OBJECTS:.o=.d)

# ---- generated sources ----
ifeq ($(PREBUILT),1)
# Sources already generated; nothing to do.
$(PARSER_CC) $(PARSER_HH) $(LEXER_CC) $(LEXER_HH):
	@test -f $@ || { echo "error: missing prebuilt $@" >&2; exit 1; }
else
# --report=all writes $(GEN_DIR)/icl_parser.output, which is where shift/reduce
# and reduce/reduce conflicts show up. A conflict is a grammar bug, so fail the
# build instead of silently letting bison pick a resolution.
# -Wall catches more than conflicts: a nonterminal or rule that no derivation
# can reach ("useless in grammar") is dead code in the .y file, which is
# exactly what -Werror=other refuses to tolerate. The grammar once carried a
# dead `ident_list` (left behind when PortGroup was rewritten to accept
# comments) and the conflict check below did not notice, because it only ever
# greps for conflicts.
BISON_FLAGS = -Wall -Werror=other --report=all

$(PARSER_CC) $(PARSER_HH) &: $(SRC_DIR)/icl/icl_parser.yy
	@mkdir -p $(GEN_DIR)
	$(BISON) $(BISON_FLAGS) --output=$(PARSER_CC) --defines=$(PARSER_HH) $<
	@if grep -q "conflict" $(GEN_DIR)/icl_parser.output 2>/dev/null; then \
	  echo "error: grammar has conflicts; see $(GEN_DIR)/icl_parser.output" >&2; \
	  exit 1; \
	fi

# flex reports "rule cannot be matched" — a scanner rule shadowed by an
# earlier one, so it can never fire. Silent today, and invisible tomorrow.
$(LEXER_CC) $(LEXER_HH) &: $(SRC_DIR)/icl/icl_lexer.l $(PARSER_HH)
	@mkdir -p $(GEN_DIR)
	$(FLEX) --outfile=$(LEXER_CC) --header-file=$(LEXER_HH) $< 2> $(GEN_DIR)/flex.log
	@if grep -qiE "warning|rule cannot be matched" $(GEN_DIR)/flex.log 2>/dev/null; then \
	  echo "error: flex reported warnings:" >&2; \
	  cat $(GEN_DIR)/flex.log >&2; \
	  exit 1; \
	fi
endif

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---- code graph ----
#
# `make codegraph` regenerates $(CODEGRAPH_DIR)/ from src/ and tests/. Both
# inputs are external tools (see below). The output is a build product and is
# gitignored — it was committed once, and every `make codegraph` then left the
# tree dirty. It is a dot-directory rather than docs/ so that it sits with the
# other generated output (build/, gen/) instead of looking like prose meant
# for a reader.
#
#   * tests_codegraph.html / .csv — `codegraph` over the Python test helpers.
#     codegraph (1.2.0) parses with Python's own tokenizer, so it only sees
#     .py files; it reports nothing for the C++ sources.
#   * cpp_deps.* — the C++ half, taken from the compiler: `g++ -MM` already
#     knows every header each translation unit includes, so there is no
#     second parser to keep in sync.
codegraph: $(ALL_OBJECTS)
	@mkdir -p $(CODEGRAPH_DIR)
	@command -v codegraph >/dev/null 2>&1 || { \
	  echo "error: codegraph not installed (pip install codegraph)" >&2; exit 1; }
	codegraph tests --output $(CODEGRAPH_DIR)/tests_codegraph.html \
	  --csv $(CODEGRAPH_DIR)/tests_codegraph.csv
	@for f in $(SOURCES); do \
	  $(CXX) $(CXXSTD) $(CPPFLAGS) $(TCL_CFLAGS) $(Z3_CFLAGS) -MM $$f; \
	done > $(CODEGRAPH_DIR)/cpp_deps.makefile
	@python3 tests/deps_to_dot.py $(CODEGRAPH_DIR)
	@echo "codegraph: $(CODEGRAPH_DIR)/ refreshed"

# ---- coverage ----
#
# Builds every binary with --coverage into $(COV_DIR), runs all three suites
# against them, then reports line and branch coverage of the C++ sources.
#
# Both numbers are reported because neither implies the other: a line counts
# as covered when it runs once, which says nothing about whether the other arm
# of its `if` was ever taken. Branch coverage is the one that finds the case a
# test forgot.
#
# Unit tests are linked against the same instrumented objects, so their counts
# merge with the end-to-end runs instead of being measured separately.
COV_DIR := $(BUILD_DIR)/cov
COV_REPORT_DIR := .worktrees/coverage
COV_CXXFLAGS := -fprofile-arcs -ftest-coverage -g -O0 -MMD -MP
-include $(wildcard $(COV_DIR)/*.d $(COV_DIR)/*/*.d)

# One set of instrumented objects, linked three ways. Each binary has its own
# `main`, so the other two mains are filtered out rather than listed by hand.
COV_SHARED := $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(SHARED_SOURCES)) \
    $(patsubst $(GEN_DIR)/%.cc,$(COV_DIR)/%.o,$(GEN_SOURCES))
COV_CLI := $(COV_SHARED) $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(CLI_SOURCES))
COV_LSP := $(COV_SHARED) $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(LSP_SOURCES))
COV_UNIT := $(COV_SHARED) $(COV_DIR)/utility/json.o $(COV_DIR)/unit_tests.o
COV_BINARIES := $(COV_DIR)/icl-format $(COV_DIR)/icl-lsp $(COV_DIR)/unittests
COV_BINARIES += $(COV_DIR)/pdl-format $(COV_DIR)/pdl-lsp
COV_PATTERN := $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(PATTERN_SOURCES))
COV_BINARIES += $(COV_DIR)/pattern_design_test $(COV_DIR)/pattern_runtime_test
COV_BINARIES += $(COV_DIR)/pattern_expression_test $(COV_DIR)/pattern_netlist_test
COV_BINARIES += $(COV_DIR)/pattern_address_test $(COV_DIR)/pattern_bsdl_test
COV_BINARIES += $(COV_DIR)/pattern_access_link_test
COV_BINARIES += $(COV_DIR)/pattern_tap_test $(COV_DIR)/pattern_state_test
COV_BINARIES += $(COV_DIR)/pattern_symbolic_test $(COV_DIR)/pattern_scheduler_test $(COV_DIR)/icl-pattern-generation
COV_BINARIES += $(COV_DIR)/pattern_logic_test $(COV_DIR)/pattern_flatten_test
COV_REPORT_OBJECTS := $(COV_CLI) $(COV_DIR)/utility/json.o \
    $(COV_DIR)/icl/icl_lsp_main.o $(COV_DIR)/unit_tests.o
COV_REPORT_OBJECTS += $(COV_DIR)/pdl/pdl_document.o $(COV_DIR)/pdl/pdl_format_main.o \
    $(COV_DIR)/pdl/pdl_lsp_main.o
COV_SOLVER := $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(SOLVER_SOURCES))
COV_REPORT_OBJECTS += $(COV_PATTERN) $(COV_SOLVER) $(COV_DIR)/pattern/icl_pattern_generation_main.o
$(COV_SOLVER): CPPFLAGS += $(Z3_CFLAGS)
$(COV_SOLVER): | check-z3
# Inline authored headers can be emitted only in a test translation unit.
COV_REPORT_OBJECTS += $(foreach test,$(filter %_test,$(COV_BINARIES)),\
    $(test)-$(notdir $(test)).gcno)

$(COV_DIR)/pdl/pdl_document.o: CPPFLAGS += $(TCL_CFLAGS)
$(COV_DIR)/pattern/pattern_runtime.o: CPPFLAGS += $(TCL_CFLAGS)

$(COV_DIR):
	mkdir -p $(COV_DIR)

$(COV_DIR)/%.o: $(SRC_DIR)/%.cc $(PARSER_HH) | $(COV_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/%.o: $(GEN_DIR)/%.cc | $(COV_DIR)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/unit_tests.o: tests/unit_tests.cc $(PARSER_HH) | $(COV_DIR)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/icl-format: $(COV_CLI)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

$(COV_DIR)/icl-lsp: $(COV_LSP)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

$(COV_DIR)/pdl-format: $(COV_SHARED) $(COV_DIR)/pdl/pdl_document.o $(COV_DIR)/pdl/pdl_format_main.o
	$(CXX) $(CXXSTD) --coverage $^ $(TCL_LIBS) -o $@

$(COV_DIR)/pdl-lsp: $(COV_SHARED) $(COV_DIR)/pdl/pdl_document.o $(COV_DIR)/pdl/pdl_lsp_main.o $(COV_DIR)/utility/json.o
	$(CXX) $(CXXSTD) --coverage $^ $(TCL_LIBS) -o $@

$(COV_DIR)/unittests: $(COV_UNIT)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

$(COV_DIR)/pattern_design_test: tests/src/pattern_design_test.cc $(COV_DIR)/pattern/pattern_design.o $(COV_DIR)/pattern/pattern_bsdl.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(COV_DIR)/pattern_flatten_test: tests/src/pattern_flatten_test.cc $(COV_DIR)/utility/json.o $(COV_PATTERN) $(COV_SHARED) $(COV_DIR)/pdl/pdl_document.o
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ $(TCL_LIBS) -o $@

$(COV_DIR)/pattern_runtime_test: tests/src/pattern_runtime_test.cc $(COV_DIR)/utility/json.o $(COV_PATTERN) $(COV_SHARED) $(COV_DIR)/pdl/pdl_document.o
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ $(TCL_LIBS) -o $@

$(COV_DIR)/pattern_expression_test: tests/src/pattern_expression_test.cc $(COV_DIR)/pattern/pattern_expression.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(COV_DIR)/pattern_netlist_test: tests/src/pattern_netlist_test.cc $(COV_DIR)/pattern/pattern_flatten.o $(COV_DIR)/utility/json.o $(COV_DIR)/pattern/pattern_logic.o $(COV_DIR)/pattern/pattern_netlist.o $(COV_DIR)/pattern/pattern_design.o $(COV_DIR)/pattern/pattern_bsdl.o $(COV_DIR)/pattern/pattern_expression.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(COV_DIR)/pattern_address_test: tests/src/pattern_address_test.cc $(COV_DIR)/pattern/pattern_address.o $(COV_DIR)/pattern/pattern_flatten.o $(COV_DIR)/utility/json.o $(COV_DIR)/pattern/pattern_logic.o $(COV_DIR)/pattern/pattern_netlist.o $(COV_DIR)/pattern/pattern_design.o $(COV_DIR)/pattern/pattern_bsdl.o $(COV_DIR)/pattern/pattern_expression.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

$(COV_DIR)/pattern_logic_test: tests/src/pattern_logic_test.cc $(COV_DIR)/pattern/pattern_flatten.o $(COV_DIR)/utility/json.o $(COV_DIR)/pattern/pattern_logic.o $(COV_DIR)/pattern/pattern_netlist.o $(COV_DIR)/pattern/pattern_design.o $(COV_DIR)/pattern/pattern_bsdl.o $(COV_DIR)/pattern/pattern_expression.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

coverage: $(COV_BINARIES)
	@mkdir -p $(COV_REPORT_DIR)
	@rm -f $(COV_DIR)/*.gcda $(COV_DIR)/*/*.gcda $(COV_REPORT_DIR)/*.gcov *.gcov
	@python3 tests/tests/test_golden_test.py
	@python3 tests/golden_test.py --binary $(COV_DIR)/icl-format --server $(COV_DIR)/icl-lsp
	@python3 tests/golden_test.py --language pdl --binary $(COV_DIR)/pdl-format --server $(COV_DIR)/pdl-lsp
	@python3 tests/src/test_format_cli.py --icl $(COV_DIR)/icl-format --pdl $(COV_DIR)/pdl-format
	@python3 tests/src/test_lsp_server.py --icl $(COV_DIR)/icl-lsp --pdl $(COV_DIR)/pdl-lsp
	@python3 tests/regression.py --binary $(COV_DIR)/icl-format
	@python3 tests/src/test_icl_conformance.py --binary $(COV_DIR)/icl-format
	@python3 tests/block_review.py --binary $(COV_DIR)/icl-format
	@python3 tests/src/test_icl_lsp.py --binary $(COV_DIR)/icl-lsp
	@python3 tests/src/test_icl_protocol.py --binary $(COV_DIR)/icl-lsp
	@python3 tests/src/test_icl_diagnostics.py --binary $(COV_DIR)/icl-lsp
	@python3 tests/src/test_pdl_tools.py --formatter $(COV_DIR)/pdl-format --server $(COV_DIR)/pdl-lsp
	@python3 tests/src/test_pdl_corpus.py --formatter $(COV_DIR)/pdl-format --server $(COV_DIR)/pdl-lsp
	@python3 tests/src/test_pdl_blocks.py --formatter $(COV_DIR)/pdl-format --server $(COV_DIR)/pdl-lsp
	@python3 tests/src/test_pdl_syntax_properties.py --formatter $(COV_DIR)/pdl-format --server $(COV_DIR)/pdl-lsp --report $(COV_DIR)/pdl-syntax-report.json
	@python3 tests/run_tests.py --binary $(COV_DIR)/icl-format
	@python3 tests/lsp_test.py --binary $(COV_DIR)/icl-lsp \
	  --cli $(COV_DIR)/icl-format
	@$(COV_DIR)/unittests
	@$(COV_DIR)/pattern_design_test
	@$(COV_DIR)/pattern_runtime_test
	@$(COV_DIR)/pattern_expression_test
	@$(COV_DIR)/pattern_bsdl_test
	@$(COV_DIR)/pattern_access_link_test
	@$(COV_DIR)/pattern_tap_test
	@$(COV_DIR)/pattern_state_test
	@$(COV_DIR)/pattern_symbolic_test
	@$(COV_DIR)/pattern_scheduler_test
	@python3 tests/src/test_icl_pattern_generation_main.py $(COV_DIR)/icl-pattern-generation
	@python3 tests/src/pattern/test_tap_pdl_replay.py $(COV_DIR)/icl-pattern-generation
	@python3 tests/src/pattern/test_apply_end_state.py $(COV_DIR)/icl-pattern-generation
	@$(COV_DIR)/pattern_netlist_test
	@python3 tests/src/test_pattern_address.py $(COV_DIR)/pattern_address_test
	@$(COV_DIR)/pattern_logic_test
	@$(COV_DIR)/pattern_flatten_test
	@gcov -b -c $(COV_REPORT_OBJECTS) > /dev/null
	@mv *.gcov $(COV_REPORT_DIR)/
	@cd $(COV_REPORT_DIR) && python3 $(abspath tests/coverage.py) --min-line 100 --min-branch 90 --strict

# ---- grammar and scanner coverage ----
#
# Measures distinct productions and scanner source-rule lines reached in
# src/icl/icl_parser.yy and src/icl/icl_lexer.l. The generated C++ is not measured —
# it is machine-written and nobody can act on its line numbers — but a rule in
# the .y/.l that no test ever reaches is a rule nobody knows works.
#
# Both numbers come from the generators themselves rather than an instrumenter
# of ours: `bison -t` prints "Reducing stack by rule N (line L)" for every
# reduction, and `flex -d` prints "--accepting rule at line N" for every token.
# The authoritative set of rule lines is read back out of the generated tables
# (yyrline[] and yy_rule_linenum[]), so the report cannot drift from the
# grammar.
GRAMCOV_DIR := $(BUILD_DIR)/gramcov
GRAMCOV_PARSER := $(GRAMCOV_DIR)/icl_parser.cc
GRAMCOV_LEXER := $(GRAMCOV_DIR)/icl_lexer.cc
GRAMCOV_BIN := $(GRAMCOV_DIR)/icl-format
GRAMCOV_SRCS := $(SHARED_SOURCES) $(CLI_SOURCES)
GRAMCOV_OBJS := $(patsubst $(SRC_DIR)/%.cc,$(GRAMCOV_DIR)/%.o,$(GRAMCOV_SRCS)) \
    $(GRAMCOV_DIR)/icl_parser.o $(GRAMCOV_DIR)/icl_lexer.o
-include $(wildcard $(GRAMCOV_DIR)/*.d $(GRAMCOV_DIR)/*/*.d)

$(GRAMCOV_DIR):
	mkdir -p $(GRAMCOV_DIR)

$(GRAMCOV_PARSER): $(SRC_DIR)/icl/icl_parser.yy | $(GRAMCOV_DIR)
	$(BISON) --output=$(GRAMCOV_PARSER) --defines=$(GRAMCOV_DIR)/icl_parser.hh $<

# bison writes the header alongside the .cc, so it has no recipe of its own;
# an empty one keeps make from concluding the target is impossible.
$(GRAMCOV_DIR)/icl_parser.hh: $(GRAMCOV_PARSER) ;
$(GRAMCOV_DIR)/icl_lexer.h: $(GRAMCOV_LEXER) ;

$(GRAMCOV_LEXER): $(SRC_DIR)/icl/icl_lexer.l $(GRAMCOV_DIR)/icl_parser.hh
	$(FLEX) -d --outfile=$(GRAMCOV_LEXER) --header-file=$(GRAMCOV_DIR)/icl_lexer.h $<

$(GRAMCOV_DIR)/%.o: $(SRC_DIR)/%.cc $(GRAMCOV_DIR)/icl_parser.hh | $(GRAMCOV_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -MMD -MP -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
	  $(CPPFLAGS) -c $< -o $@

$(GRAMCOV_DIR)/icl_parser.o: $(GRAMCOV_PARSER) | $(GRAMCOV_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -MMD -MP -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
	  $(CPPFLAGS) -c $< -o $@

$(GRAMCOV_DIR)/icl_lexer.o: $(GRAMCOV_LEXER) | $(GRAMCOV_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -MMD -MP -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
	  $(CPPFLAGS) -c $< -o $@

# Also replace old objects built before header dependency tracking was enabled.
$(GRAMCOV_OBJS): Makefile

$(GRAMCOV_BIN): $(GRAMCOV_OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

gramcov: $(GRAMCOV_BIN)
	@python3 tests/grammar_coverage.py --binary $(GRAMCOV_BIN) \
	  --parser $(GRAMCOV_PARSER) --lexer $(GRAMCOV_LEXER) \
	  --report $(GRAMCOV_DIR)/blocks.json

# ---- installation ----
#
# `make install` writes a manifest so that `make uninstall` removes exactly the
# files that were installed — no guessing, and no deleting anything that was
# already in the target directory.
MANIFEST := $(BUILD_DIR)/install_manifest.txt

# Both install targets put the programs under the *same* names, so the choice
# is purely "which build do I want on PATH" — an editor config that says
# `"command": "icl-lsp"` works either way. `make` builds both; installing is a
# separate decision.
#
#   install-dynamic  smaller; needs the host's libstdc++/glibc
#   install-static   no ELF shared-library dependencies; see Tcl/glibc limits in docs
#
# Because both write the same paths, the manifest is correct whichever ran.
HEADERS := $(wildcard $(SRC_DIR)/*/*.h)

install: install-dynamic

# One recipe body, parameterised by (formatter, lsp, label). The two targets
# differ only in which files they install — everything else, manifest included,
# is identical, so it is written once.
define install_body
	@mkdir -p $(bindir) $(includedir)/$(PACKAGE) $(docdir) $(man1dir)
	$(INSTALL_PROGRAM) $1 $(bindir)/$(PACKAGE)
	$(INSTALL_PROGRAM) $2 $(bindir)/icl-lsp
	$(INSTALL_PROGRAM) $(if $(filter static,$3),$(PDL_STATIC_BIN),$(PDL_BIN)) $(bindir)/pdl-format
	$(INSTALL_PROGRAM) $(if $(filter static,$3),$(PDL_LSP_STATIC_BIN),$(PDL_LSP_BIN)) $(bindir)/pdl-lsp
	$(INSTALL_PROGRAM) $(if $(filter static,$3),$(PATTERN_GENERATOR_STATIC),$(PATTERN_GENERATOR)) $(bindir)/icl-pattern-generation
	$(INSTALL) -d $(docdir)/licenses
	$(INSTALL_DATA) docs/licenses/Z3.txt $(docdir)/licenses/Z3.txt
	@for h in $(HEADERS); do \
	  relative=$${h#$(SRC_DIR)/}; \
	  $(INSTALL) -d "$(includedir)/$(PACKAGE)/$${relative%/*}" || exit $$?; \
	  $(INSTALL_DATA) "$$h" "$(includedir)/$(PACKAGE)/$$relative" || exit $$?; \
	  previous="$(includedir)/$(PACKAGE)/$${h##*/}"; \
	  if [ -f "$(MANIFEST)" ] && grep -Fxq -- "$$previous" "$(MANIFEST)"; then \
	    rm -f "$$previous" || exit $$?; \
	  fi; \
	done
	$(INSTALL_DATA) docs/third-party-notices.md $(docdir)/third-party-notices.md
	@if [ -f $(PACKAGE).1 ]; then $(INSTALL_DATA) $(PACKAGE).1 $(man1dir)/; fi
	@: > $(MANIFEST)
	@echo $(bindir)/$(PACKAGE) >> $(MANIFEST)
	@echo $(bindir)/icl-lsp >> $(MANIFEST)
	@echo $(bindir)/pdl-format >> $(MANIFEST)
	@echo $(bindir)/pdl-lsp >> $(MANIFEST)
	@echo $(bindir)/icl-pattern-generation >> $(MANIFEST)
	@echo $(docdir)/licenses/Z3.txt >> $(MANIFEST)
	@echo $(docdir)/third-party-notices.md >> $(MANIFEST)
	@for h in $(HEADERS); do \
	  echo $(includedir)/$(PACKAGE)/$${h#$(SRC_DIR)/} >> $(MANIFEST); \
	done
	@if [ -f $(PACKAGE).1 ]; then echo $(man1dir)/$(PACKAGE).1 >> $(MANIFEST); fi
	@echo "installed ($3) icl-format, icl-lsp, pdl-format, pdl-lsp, icl-pattern-generation $(VERSION) into $(prefix)"
	@$(call coc_body,$3)
endef

# One server supplies both language services and editor formatting.
#
# `setlocal filetype=icl` — not `setfiletype` — is deliberate: vim's own filetype.vim
# maps *.icl to the Clean language, and setfiletype silently does nothing once
# a filetype is already set, which leaves coc never starting the server.
define coc_body
	@echo ""
	@echo "=== coc.nvim: ICL and PDL language services and formatting ==="
	@echo "In :CocConfig, merge icl and pdl into languageserver alongside your existing servers:"
	@cat docs/editor/coc-lsp.json
	@echo ""
	@echo "Add to vimrc:"
	@echo '  autocmd BufNewFile,BufRead *.icl setlocal filetype=icl'
	@echo '  autocmd BufNewFile,BufRead *.pdl setlocal filetype=pdl'
	@echo "  autocmd FileType pdl if empty(globpath(&runtimepath, 'syntax/pdl.vim')) | setlocal syntax=tcl | endif"
	@echo "Indentation follows your existing Vim/coc shiftwidth and tabstop settings."
	@echo "Formatting: :call CocAction('format'); optional format on save:"
	@echo "  autocmd BufWritePre *.icl call CocAction('format')"
	@echo "  autocmd BufWritePre *.pdl call CocAction('format')"
	@echo ""
	@echo "icl-lsp includes the icl-format formatting engine; no extra formatter plugin is needed."
	@echo "Standalone CLI formatting: icl-format input.icl > output.icl"
	@echo "PDL formatting: pdl-format input.pdl > output.pdl; PDL analysis never executes source."
	@echo "Keep $(bindir) on the editor's PATH or use absolute command paths."
	@echo "Installed variant: $1; $(bindir)/$(PACKAGE) and $(bindir)/icl-lsp"
endef

install-dynamic: $(BIN) $(LSP_BIN) $(PDL_BIN) $(PDL_LSP_BIN)
	$(call install_body,$(BIN),$(LSP_BIN),dynamic)
	@echo "for fully static ELF binaries: make install-static (see docs/pdl-tools.md for platform limits)"

install-static: $(STATIC_BIN) $(LSP_STATIC_BIN) $(PDL_STATIC_BIN) $(PDL_LSP_STATIC_BIN)
	$(call install_body,$(STATIC_BIN),$(LSP_STATIC_BIN),static)

uninstall:
	@if [ ! -f $(MANIFEST) ]; then \
	  echo "no manifest at $(MANIFEST); removing known targets instead" >&2; \
	  rm -f $(bindir)/$(PACKAGE); \
	  rm -f $(bindir)/icl-lsp; \
	  rm -f $(bindir)/pdl-format $(bindir)/pdl-lsp $(bindir)/icl-pattern-generation $(bindir)/icl-pattern-generation-static; \
	  rm -rf $(includedir)/$(PACKAGE); \
	  rm -rf $(docdir); \
	  rm -f $(man1dir)/$(PACKAGE).1; \
	else \
	  while IFS= read -r file; do \
	    [ -n "$$file" ] && rm -f "$$file"; \
	  done < $(MANIFEST); \
	  rm -f $(MANIFEST); \
	fi
	@# Remove directories that are now empty, but never $(prefix) itself.
	@-rmdir $(addprefix $(includedir)/$(PACKAGE)/,icl pdl pattern utility) $(includedir)/$(PACKAGE) $(docdir) 2>/dev/null || true
	@echo "uninstalled $(PACKAGE) from $(prefix)"

# Optional: exercise the server from neovim's built-in LSP client. This is a
# real client doing the real handshake, not a scripted replay, so it catches
# things a hand-written test can miss (it is how the tabSize behaviour below was
# found). Skipped when nvim is absent.
nvimcheck: $(LSP_BIN)
	@if command -v nvim >/dev/null 2>&1; then \
	  LSPBIN=$(CURDIR)/$(LSP_BIN) nvim --headless -u NONE \
	    -c "luafile tests/nvim_lsp_check.lua"; \
	else \
	  echo "nvim not installed; skipping the live client check"; \
	fi

# Optional: drive the server from vim through coc.nvim — the full chain
# (vim -> coc.nvim -> icl-lsp) with nothing simulated. Needs coc.nvim, which
# setup_coc.sh fetches, and vim 9+.
#
# This is how three real problems were found that no unit test could see: coc
# needs vim 9.2 (vim 8.2 rejects its vim9script), `setfiletype` loses to vim's
# built-in `*.icl -> clean` mapping, and the editor's shiftwidth is what
# decides indent width. Skipped when the pieces are absent.
coccheck: $(LSP_BIN)
	@if [ ! -d "$(COC_PLUGIN)" ]; then \
	  echo "coc.nvim not prepared; run tests/setup_coc.sh first"; \
	else \
	  COC_ROOT=$(COC_ROOT) COC_PLUGIN=$(COC_PLUGIN) COC_VIM=$(COC_VIM) \
	    bash tests/run_coc_check.sh; \
	fi

# The corpus in tests/cases/ runs first: it is the cheapest way to see a
# regression, and a failure there names the file that broke.
check: all
check: pattern-runtime-static-check pattern-flatten-static-check
check-dynamic: dynamic
check check-dynamic: unitcheck corpuscheck syntaxcheck keywordcheck pdlcheck goldencheck pattern-design-check
check check-dynamic: pattern-runtime-check pattern-flatten-check
check check-dynamic: tessent-oracle-check tessent-golden-check
check check-dynamic: pattern-expression-check pattern-netlist-check pattern-logic-check pattern-netlist-benchmark
check check-dynamic:
	@python3 tests/tree_clean_test.py
	@python3 tests/pdl_separation_test.py --binary $(BIN)
	@python3 tests/src/test_format_cli.py --icl $(BIN) --pdl $(PDL_BIN)
	@python3 tests/src/test_lsp_server.py --icl $(LSP_BIN) --pdl $(PDL_LSP_BIN)
	@python3 tests/src/test_icl_semantics.py --binary $(BIN)
	@python3 tests/src/test_pdl_document.py --server $(PDL_LSP_BIN)
	@python3 tests/regression.py --binary $(BIN)
	@python3 tests/src/test_icl_conformance.py --binary $(BIN)
	@python3 tests/tests/test_antlr_diff.py
	@python3 tests/tests/test_coverage.py
	@python3 tests/tests/test_grammar_coverage.py
	@python3 tests/tests/test_block_review.py
	@python3 tests/tests/test_regression.py
	@python3 tests/tests/test_makefile.py
	@python3 tests/tests/test_deps_to_dot.py
	@python3 tests/tests/src/test_probe_pattern_access_link_connections.py
	@python3 tests/tests/test_tree_clean_test.py
	@python3 tests/whitespace_test.py --binary $(BIN)
	@python3 tests/fuzz.py --binary $(BIN)
	@python3 tests/block_review.py --binary $(BIN)
	@python3 tests/run_tests.py --binary $(BIN)
	@python3 tests/lsp_test.py --binary $(LSP_BIN) --cli $(BIN)
	@python3 tests/src/test_icl_lsp.py --binary $(LSP_BIN)
	@python3 tests/src/test_icl_protocol.py --binary $(LSP_BIN)
	@python3 tests/src/test_icl_diagnostics.py --binary $(LSP_BIN)
	@$(MAKE) --no-print-directory gramcov
	@$(MAKE) --no-print-directory vim-plugin-check

COC_ROOT ?= /tmp/cocvim
COC_PLUGIN ?= $(COC_ROOT)/coc.nvim
COC_VIM ?= /usr/local/vim9/bin/vim

# Parser-backed inventory is test support, not an installed executable.
.PHONY: tessent-oracle-check
tessent-oracle-check: $(BUILD_DIR)/pdl_pattern_trace $(BUILD_DIR)/icl_corpus_inventory
	@python3 tests/tests/test_tessent_oracle.py
	@PDL_TRACE=$(BUILD_DIR)/pdl_pattern_trace python3 tests/src/test_pdl_pattern_trace.py

.PHONY: tessent-golden-check
tessent-golden-check: $(BIN) $(PDL_BIN) $(BUILD_DIR)/icl-pattern-generation $(BUILD_DIR)/pdl_pattern_trace
	@GOLDEN_BUILD=$(BUILD_DIR) python3 tests/tests/test_tessent_golden.py
	@python3 tests/tessent_golden.py check --build $(BUILD_DIR)
	@GOLDEN_BUILD=$(BUILD_DIR) python3 tests/tests/test_tessent_roundtrip.py
	@python3 tests/tessent_roundtrip.py check --build $(BUILD_DIR)

$(BUILD_DIR)/pdl_pattern_trace: tests/src/pdl_pattern_trace.cc \
    $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/utility/json.o \
    $(PATTERN_OBJECTS) $(BUILD_DIR)/pattern/pattern_values.o | check-z3
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $(Z3_CFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@

$(BUILD_DIR)/icl_corpus_inventory: tests/icl_corpus_inventory.cc \
    $(SHARED_OBJECTS) $(BUILD_DIR)/utility/json.o
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

# Acceptance-preserving IEEE transformations, independent of our .yy/.l rules.
syntaxcheck: $(BUILD_DIR)/icl_corpus_inventory
	python3 tests/tests/src/test_icl_metamorphic.py
	python3 tests/src/test_icl_metamorphic.py --inventory $<

# Cross-context identifier permissions are independent of our token inventory.
keywordcheck: $(BIN) $(LSP_BIN) $(BUILD_DIR)/icl_corpus_inventory
	python3 tests/tests/src/test_icl_keyword_contexts.py
	python3 tests/src/test_icl_keyword_contexts.py --binary $(BIN) \
	  --lsp $(LSP_BIN) --inventory $(BUILD_DIR)/icl_corpus_inventory

.PHONY: corpuscheck
corpuscheck: $(BUILD_DIR)/icl_corpus_inventory $(BIN)
	python3 tests/tests/test_import_eda_corpus.py
	python3 tests/tests/test_eda_corpus.py
	python3 tests/tests/test_audit_icl_corpus.py
	python3 tests/audit_icl_corpus.py --policy tests/cases/deduplication.json

# ---- unit tests ----
#
# The AST and the layout pass take plain line numbers (not Bison's location
# type), so they can be tested without generating a parser. That independence
# is deliberate: it is what makes the two passes separately verifiable.
UNIT_BIN := $(BUILD_DIR)/unit_tests

unitcheck: $(UNIT_BIN)
	@$(UNIT_BIN)

.PHONY: pattern-design-check
pattern-design-check: $(BUILD_DIR)/pattern_design_test
	@$(BUILD_DIR)/pattern_design_test

$(BUILD_DIR)/pattern_design_test: tests/src/pattern_design_test.cc $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

.PHONY: pattern-expression-check
pattern-expression-check: $(BUILD_DIR)/pattern_expression_test
	@$(BUILD_DIR)/pattern_expression_test

$(BUILD_DIR)/pattern_expression_test: tests/src/pattern_expression_test.cc $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

.PHONY: pattern-netlist-check
pattern-netlist-check: $(BUILD_DIR)/pattern_netlist_test
	@$(BUILD_DIR)/pattern_netlist_test

$(BUILD_DIR)/pattern_netlist_test: tests/src/pattern_netlist_test.cc $(BUILD_DIR)/pattern/pattern_flatten.o $(BUILD_DIR)/utility/json.o $(BUILD_DIR)/pattern/pattern_logic.o $(BUILD_DIR)/pattern/pattern_netlist.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

.PHONY: pattern-netlist-benchmark
pattern-netlist-benchmark: $(BUILD_DIR)/pattern_netlist_benchmark
	@python3 tests/src/test_pattern_netlist_benchmark.py $(BUILD_DIR)/pattern_netlist_benchmark

$(BUILD_DIR)/pattern_netlist_benchmark: tests/src/pattern_netlist_benchmark.cc $(BUILD_DIR)/pattern/pattern_flatten.o $(BUILD_DIR)/utility/json.o $(BUILD_DIR)/pattern/pattern_logic.o $(BUILD_DIR)/pattern/pattern_netlist.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

.PHONY: pattern-logic-check
pattern-logic-check: $(BUILD_DIR)/pattern_logic_test
	@$(BUILD_DIR)/pattern_logic_test

$(BUILD_DIR)/pattern_logic_test: tests/src/pattern_logic_test.cc $(BUILD_DIR)/pattern/pattern_flatten.o $(BUILD_DIR)/utility/json.o $(BUILD_DIR)/pattern/pattern_logic.o $(BUILD_DIR)/pattern/pattern_netlist.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

.PHONY: pattern-runtime-check pattern-runtime-static-check
pattern-runtime-check: $(BUILD_DIR)/pattern_runtime_test
	@$(BUILD_DIR)/pattern_runtime_test

$(BUILD_DIR)/pattern_runtime_test: tests/src/pattern_runtime_test.cc $(BUILD_DIR)/utility/json.o $(PATTERN_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ $(TCL_LIBS) -o $@

pattern-runtime-static-check: $(BUILD_DIR)/pattern_runtime_test-static
	@$(BUILD_DIR)/pattern_runtime_test-static

$(BUILD_DIR)/pattern_runtime_test-static: tests/src/pattern_runtime_test.cc $(BUILD_DIR)/utility/json.o $(PATTERN_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT) | check-tcl-static
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ $(TCL_STATIC_LIBS) -o $@

$(UNIT_BIN): tests/unit_tests.cc $(SRC_DIR)/icl/format_tree.cc \
    $(SRC_DIR)/utility/json.cc $(SRC_DIR)/icl/icl_ast.h $(SRC_DIR)/icl/format_tree.h \
    $(SRC_DIR)/utility/json.h $(SRC_DIR)/utility/utf8.h $(SRC_DIR)/utility/text_position.h
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) \
	  tests/unit_tests.cc $(SRC_DIR)/icl/format_tree.cc $(SRC_DIR)/utility/json.cc -o $@

# ---- source distribution with the parser pre-generated ----
#
# Lets a machine without bison/flex (e.g. stock Rocky Linux 8, which ships
# bison 3.0.4) build with g++ and Tcl development files. The tarball contains the generated
# parser and scanner sources plus a stub Makefile that skips the generators.
DIST_DIR := $(PACKAGE)-$(VERSION)
# The tarball is a build product, so it goes under build/ rather than next to
# the sources. It used to land in the source root, where it was then swept up
# by anything that archived the tree — a hand-me-down tarball containing
# another tarball, more than half its size. build/ is excluded from source
# archives and cleaned by `distclean`, so this cannot happen again.
DIST_OUT := $(BUILD_DIR)/dist

dist: dynamic
	@rm -rf $(DIST_DIR)
	@mkdir -p $(DIST_OUT)
	@mkdir -p $(DIST_DIR)/src $(DIST_DIR)/tests $(DIST_DIR)/gen
	cp --parents $(SOURCES) $(HEADERS) $(SRC_DIR)/icl/icl_parser.yy $(SRC_DIR)/icl/icl_lexer.l $(DIST_DIR)/
	cp $(PARSER_CC) $(PARSER_HH) $(LEXER_CC) $(LEXER_HH) $(DIST_DIR)/gen/
	cp $(GEN_DIR)/location.hh $(DIST_DIR)/gen/ 2>/dev/null || true
	# Recursive: the corpus is sorted into per-source subdirectories, and a
	# flat copy would ship an empty set while appearing to succeed.
	cp -r tests/cases tests/golden tests/src tests/tests tests/vim-icl-pdl-modern $(DIST_DIR)/tests/
	# unit_tests.cc is part of `make check` (the `unitcheck` target builds it),
	# so a tarball without it cannot run its own test suite. The copy list was
	# once only .py/.vim/.sh/.lua, and `make dist` still reported success.
	cp tests/*.py tests/*.cc tests/*.vim tests/*.sh tests/*.lua $(DIST_DIR)/tests/
	cp Makefile README.md LICENSE .clang-format .clang-tidy $(DIST_DIR)/
	cp -r docs $(DIST_DIR)/
	cp -r vim-icl-pdl-modern $(DIST_DIR)/
	@echo "PREBUILT = 1" > $(DIST_DIR)/.prebuilt
	tar --exclude=__pycache__ --exclude='*.pyc' -czf $(DIST_OUT)/$(DIST_DIR).tar.gz $(DIST_DIR)
	@rm -rf $(DIST_DIR)
	@echo "created $(DIST_OUT)/$(DIST_DIR).tar.gz (build with g++ and Tcl: no bison/flex needed)"

version:
	@echo $(VERSION)

clean:
	rm -rf $(BUILD_DIR) $(COV_REPORT_DIR)

distclean: clean
	rm -rf $(GEN_DIR) $(DIST_OUT) $(DIST_DIR) $(DIST_DIR).tar.gz

.PHONY: pattern-flatten-check
pattern-flatten-check: $(BUILD_DIR)/pattern_flatten_test
	@$(BUILD_DIR)/pattern_flatten_test

$(BUILD_DIR)/pattern_flatten_test: tests/src/pattern_flatten_test.cc $(BUILD_DIR)/utility/json.o $(PATTERN_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ $(TCL_LIBS) -o $@

.PHONY: pattern-flatten-static-check
pattern-flatten-static-check: $(BUILD_DIR)/pattern_flatten_test-static
	@$(BUILD_DIR)/pattern_flatten_test-static

$(BUILD_DIR)/pattern_flatten_test-static: tests/src/pattern_flatten_test.cc $(BUILD_DIR)/utility/json.o $(PATTERN_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT) | check-tcl-static
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ $(TCL_STATIC_LIBS) -o $@

.PHONY: pattern-address-check pattern-address-static-check
pattern-address-check: $(BUILD_DIR)/pattern_address_test
	@python3 tests/src/test_pattern_address.py $(BUILD_DIR)/pattern_address_test

pattern-address-static-check: $(BUILD_DIR)/pattern_address_test-static
	@python3 tests/src/test_pattern_address.py $(BUILD_DIR)/pattern_address_test-static

$(BUILD_DIR)/pattern_address_test: tests/src/pattern_address_test.cc $(BUILD_DIR)/pattern/pattern_address.o $(BUILD_DIR)/pattern/pattern_flatten.o $(BUILD_DIR)/utility/json.o $(BUILD_DIR)/pattern/pattern_logic.o $(BUILD_DIR)/pattern/pattern_netlist.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR)/pattern_address_test-static: tests/src/pattern_address_test.cc $(BUILD_DIR)/pattern/pattern_address.o $(BUILD_DIR)/pattern/pattern_flatten.o $(BUILD_DIR)/utility/json.o $(BUILD_DIR)/pattern/pattern_logic.o $(BUILD_DIR)/pattern/pattern_netlist.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(BUILD_DIR)/pattern/pattern_expression.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ -o $@

check check-dynamic: pattern-address-check
check check-static: pattern-address-static-check

.PHONY: pattern-bsdl-check pattern-bsdl-static-check
pattern-bsdl-check: $(BUILD_DIR)/pattern_bsdl_test
	@$(BUILD_DIR)/pattern_bsdl_test

pattern-bsdl-static-check: $(BUILD_DIR)/pattern_bsdl_test-static
	@$(BUILD_DIR)/pattern_bsdl_test-static

$(BUILD_DIR)/pattern_bsdl_test: tests/src/pattern_bsdl_test.cc $(BUILD_DIR)/pattern/pattern_bsdl.o
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR)/pattern_bsdl_test-static: tests/src/pattern_bsdl_test.cc $(BUILD_DIR)/pattern/pattern_bsdl.o
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ -o $@

$(COV_DIR)/pattern_bsdl_test: tests/src/pattern_bsdl_test.cc $(COV_DIR)/pattern/pattern_bsdl.o
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

check check-dynamic: pattern-bsdl-check
check check-static: pattern-bsdl-static-check

.PHONY: pattern-access-link-check pattern-access-link-static-check
pattern-access-link-check: $(BUILD_DIR)/pattern_access_link_test
	@$(BUILD_DIR)/pattern_access_link_test

pattern-access-link-static-check: $(BUILD_DIR)/pattern_access_link_test-static
	@$(BUILD_DIR)/pattern_access_link_test-static

$(BUILD_DIR)/pattern_access_link_test: tests/src/pattern_access_link_test.cc $(BUILD_DIR)/pattern/pattern_access_link.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR)/pattern_access_link_test-static: tests/src/pattern_access_link_test.cc $(BUILD_DIR)/pattern/pattern_access_link.o $(BUILD_DIR)/pattern/pattern_design.o $(BUILD_DIR)/pattern/pattern_bsdl.o $(SHARED_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ -o $@

$(COV_DIR)/pattern_access_link_test: tests/src/pattern_access_link_test.cc $(COV_DIR)/pattern/pattern_access_link.o $(COV_DIR)/pattern/pattern_design.o $(COV_DIR)/pattern/pattern_bsdl.o $(COV_SHARED)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

check check-dynamic: pattern-access-link-check
check check-static: pattern-access-link-static-check

.PHONY: pattern-tap-check pattern-tap-static-check
pattern-tap-check: $(BUILD_DIR)/pattern_tap_test
	@$(BUILD_DIR)/pattern_tap_test

pattern-tap-static-check: $(BUILD_DIR)/pattern_tap_test-static
	@$(BUILD_DIR)/pattern_tap_test-static

TAP_TEST_OBJECTS := $(filter-out $(BUILD_DIR)/pattern/pattern_runtime.o,$(PATTERN_OBJECTS)) $(SHARED_OBJECTS) $(BUILD_DIR)/utility/json.o

$(BUILD_DIR)/pattern_tap_test: tests/src/pattern_tap_test.cc $(TAP_TEST_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR)/pattern_tap_test-static: tests/src/pattern_tap_test.cc $(TAP_TEST_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ -o $@

$(COV_DIR)/pattern_tap_test: tests/src/pattern_tap_test.cc $(filter-out $(COV_DIR)/pattern/pattern_runtime.o,$(COV_PATTERN)) $(COV_SHARED) $(COV_DIR)/utility/json.o
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@

check check-dynamic: pattern-tap-check
check check-static: pattern-tap-static-check

.PHONY: pattern-state-check pattern-state-static-check
pattern-state-check: $(BUILD_DIR)/pattern_state_test
	@$(BUILD_DIR)/pattern_state_test
pattern-state-static-check: $(BUILD_DIR)/pattern_state_test-static
	@$(BUILD_DIR)/pattern_state_test-static
$(BUILD_DIR)/pattern_state_test: tests/src/pattern_state_test.cc $(TAP_TEST_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $^ -o $@
$(BUILD_DIR)/pattern_state_test-static: tests/src/pattern_state_test.cc $(TAP_TEST_OBJECTS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -static $^ -o $@
$(COV_DIR)/pattern_state_test: tests/src/pattern_state_test.cc $(filter-out $(COV_DIR)/pattern/pattern_runtime.o,$(COV_PATTERN)) $(COV_SHARED) $(COV_DIR)/utility/json.o
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $^ -o $@
check check-dynamic: pattern-state-check
check check-static: pattern-state-static-check

.PHONY: check-z3 z3-deps pattern-symbolic-check pattern-symbolic-static-check
check-z3:
	@if [ -z "$(Z3_LIBS)" ]; then \
	  echo "error: install Z3 development files, set Z3_CFLAGS/Z3_LIBS, or run make z3-deps" >&2; \
	  exit 1; \
	fi

# Explicit, pinned dependency bootstrap; downloaded/generated files stay in build.
z3-deps:
	mkdir -p $(BUILD_DIR)/deps
	curl -L --fail https://github.com/Z3Prover/z3/archive/refs/tags/z3-$(Z3_VERSION).tar.gz -o $(BUILD_DIR)/deps/z3-$(Z3_VERSION).tar.gz
	printf '%s  %s\n' f59c9cf600ea57fb64ffeffbffd0f2d2b896854f339e846f48f069d23bc14ba0 $(BUILD_DIR)/deps/z3-$(Z3_VERSION).tar.gz | sha256sum --check --status
	tar -xzf $(BUILD_DIR)/deps/z3-$(Z3_VERSION).tar.gz -C $(BUILD_DIR)/deps
	cmake -S $(BUILD_DIR)/deps/z3-z3-$(Z3_VERSION) -B $(BUILD_DIR)/deps/z3-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE='-O2 -DNDEBUG' -DCMAKE_INSTALL_PREFIX=$(abspath $(Z3_PREFIX)) -DCMAKE_INSTALL_LIBDIR=lib -DZ3_BUILD_LIBZ3_SHARED=OFF -DZ3_BUILD_EXECUTABLE=OFF -DZ3_BUILD_TEST_EXECUTABLES=OFF -DZ3_ENABLE_EXAMPLE_TARGETS=OFF
	cmake --build $(BUILD_DIR)/deps/z3-build --parallel 2
	cmake --install $(BUILD_DIR)/deps/z3-build

pattern-symbolic-check: $(BUILD_DIR)/pattern_symbolic_test
	@$(BUILD_DIR)/pattern_symbolic_test
pattern-symbolic-static-check: $(BUILD_DIR)/pattern_symbolic_test-static
	@$(BUILD_DIR)/pattern_symbolic_test-static
$(BUILD_DIR)/pattern_symbolic_test: tests/src/pattern_symbolic_test.cc $(SOLVER_OBJECTS) $(TAP_TEST_OBJECTS) $(BUILD_DIR)/pdl/pdl_document.o | check-z3
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $(Z3_CFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@
$(BUILD_DIR)/pattern_symbolic_test-static: tests/src/pattern_symbolic_test.cc $(SOLVER_OBJECTS) $(TAP_TEST_OBJECTS) $(BUILD_DIR)/pdl/pdl_document.o | check-z3 check-tcl-static
	@if [ -z "$(Z3_STATIC_LIB)" ]; then echo 'error: static Z3 archive required; run make z3-deps' >&2; exit 1; fi
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $(Z3_CFLAGS) -static $^ $(TCL_STATIC_LIBS) $(Z3_STATIC_LIB) -lpthread -o $@
$(COV_DIR)/pattern_symbolic_test: tests/src/pattern_symbolic_test.cc $(COV_SOLVER) $(filter-out $(COV_DIR)/pattern/pattern_runtime.o,$(COV_PATTERN)) $(COV_SHARED) $(COV_DIR)/utility/json.o $(COV_DIR)/pdl/pdl_document.o | check-z3
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $(Z3_CFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@
check check-dynamic: pattern-symbolic-check
check check-static: pattern-symbolic-static-check

.PHONY: pattern-scheduler-check
pattern-scheduler-check: $(BUILD_DIR)/pattern_scheduler_test
	@$(BUILD_DIR)/pattern_scheduler_test
$(BUILD_DIR)/pattern_scheduler_test: tests/src/pattern_scheduler_test.cc $(SOLVER_OBJECTS) $(TAP_TEST_OBJECTS) $(BUILD_DIR)/pdl/pdl_document.o | check-z3
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $(Z3_CFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@

PATTERN_GENERATOR := $(BUILD_DIR)/icl-pattern-generation
PATTERN_GENERATOR_STATIC := $(PATTERN_GENERATOR)-static
$(BUILD_DIR)/pattern/icl_pattern_generation_main.o: CPPFLAGS += $(TCL_CFLAGS) $(Z3_CFLAGS)
$(PATTERN_GENERATOR): $(BUILD_DIR)/pattern/icl_pattern_generation_main.o $(PATTERN_OBJECTS) $(SOLVER_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/utility/json.o | check-z3 check-tcl
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@
$(PATTERN_GENERATOR_STATIC): $(BUILD_DIR)/pattern/icl_pattern_generation_main.o $(PATTERN_OBJECTS) $(SOLVER_OBJECTS) $(SHARED_OBJECTS) $(PDL_OBJECT) $(BUILD_DIR)/utility/json.o | check-z3 check-tcl-static
	@if [ -z "$(Z3_STATIC_LIB)" ]; then echo 'error: static Z3 archive required; run make z3-deps' >&2; exit 1; fi
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ $(TCL_STATIC_LIBS) $(Z3_STATIC_LIB) -lpthread -o $@
.PHONY: pattern-generation-check pattern-generation-static-check
pattern-generation-check: $(PATTERN_GENERATOR)
	python3 tests/src/test_icl_pattern_generation_main.py $(PATTERN_GENERATOR)
	python3 tests/src/pattern/test_dependency_scheduling.py $(PATTERN_GENERATOR)
	python3 tests/src/pattern/test_merge_scheduling.py $(PATTERN_GENERATOR)
	python3 tests/src/pattern/test_resource_reservations.py $(PATTERN_GENERATOR)
	python3 tests/src/pattern/test_tap_pdl_replay.py $(PATTERN_GENERATOR)
	python3 tests/src/pattern/test_apply_end_state.py $(PATTERN_GENERATOR)
pattern-generation-static-check: $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/test_icl_pattern_generation_main.py $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/pattern/test_dependency_scheduling.py $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/pattern/test_merge_scheduling.py $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/pattern/test_resource_reservations.py $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/pattern/test_tap_pdl_replay.py $(PATTERN_GENERATOR_STATIC)
	python3 tests/src/pattern/test_apply_end_state.py $(PATTERN_GENERATOR_STATIC)

$(BUILD_DIR)/pattern_scheduler_test-static: tests/src/pattern_scheduler_test.cc $(SOLVER_OBJECTS) $(TAP_TEST_OBJECTS) $(BUILD_DIR)/pdl/pdl_document.o | check-z3 check-tcl-static
	@if [ -z "$(Z3_STATIC_LIB)" ]; then echo 'error: static Z3 archive required; run make z3-deps' >&2; exit 1; fi
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) $(Z3_CFLAGS) -static $^ $(TCL_STATIC_LIBS) $(Z3_STATIC_LIB) -lpthread -o $@
.PHONY: pattern-scheduler-static-check
pattern-scheduler-static-check: $(BUILD_DIR)/pattern_scheduler_test-static
	@$(BUILD_DIR)/pattern_scheduler_test-static
$(COV_DIR)/pattern_scheduler_test: tests/src/pattern_scheduler_test.cc $(COV_SOLVER) $(filter-out $(COV_DIR)/pattern/pattern_runtime.o,$(COV_PATTERN)) $(COV_SHARED) $(COV_DIR)/utility/json.o $(COV_DIR)/pdl/pdl_document.o | check-z3
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) $(Z3_CFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@
$(COV_DIR)/pattern/icl_pattern_generation_main.o: CPPFLAGS += $(TCL_CFLAGS) $(Z3_CFLAGS)
$(COV_DIR)/icl-pattern-generation: $(COV_DIR)/pattern/icl_pattern_generation_main.o $(COV_PATTERN) $(COV_SOLVER) $(COV_SHARED) $(COV_DIR)/utility/json.o $(COV_DIR)/pdl/pdl_document.o | check-z3
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $^ $(TCL_LIBS) $(Z3_LIBS) -o $@
check check-dynamic: pattern-scheduler-check pattern-generation-check
check check-static: pattern-scheduler-static-check pattern-generation-static-check
dynamic: $(PATTERN_GENERATOR)
static: $(PATTERN_GENERATOR_STATIC)

install-dynamic: $(PATTERN_GENERATOR)
install-static: $(PATTERN_GENERATOR_STATIC)
