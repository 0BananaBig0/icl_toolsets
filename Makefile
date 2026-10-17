# Copyright 2024 The ICL Format Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Builds two programs for IEEE 1687-2014 (IJTAG) ICL:
#   icl-format  command line formatter
#   icl-lsp     Language Server (diagnostics + formatting) for editors
# Each is produced dynamically linked and fully static, so `make` yields four
# binaries.
#
# Pipeline:  text --(flex/bison)--> AST --(layout)--> lines --(render)--> text
# Both programs link the same front end, so a grammar fix reaches both.
#
# Targets:
#   make            build all four binaries
#   make dynamic    build only the dynamically linked pair
#   make install    install into $(prefix) (default: $HOME/.local)
#   make uninstall  remove exactly what install put there
#   make check      run the test suite
#   make clean      remove build output (keeps bison/flex output)
#   make distclean  remove everything generated
#
# Requirements: bison >= 3.8, flex >= 2.6, a C++17 compiler.
# Verified on Rocky Linux 8+ and Ubuntu 22.04+.

CXX      ?= g++
BISON    ?= bison
FLEX     ?= flex

PACKAGE   := icl-format
VERSION   := 1.0.0

# ---- installation layout (GNU style, rooted at $HOME/.local by default) ----
prefix        ?= $(HOME)/.local
exec_prefix   ?= $(prefix)
bindir        ?= $(exec_prefix)/bin
libdir        ?= $(exec_prefix)/lib
includedir    ?= $(prefix)/include
datarootdir   ?= $(prefix)/share
docdir        ?= $(datarootdir)/doc/$(PACKAGE)
mandir        ?= $(datarootdir)/man
man1dir       ?= $(mandir)/man1

INSTALL      ?= install
INSTALL_PROGRAM ?= $(INSTALL) -m 0755
INSTALL_DATA    ?= $(INSTALL) -m 0644

# ---- build layout ----
SRC_DIR   := src
BUILD_DIR := build

# When `.prebuilt` is present the generated parser/scanner are shipped in ./gen
# and bison/flex are not needed at all (see the `dist` target). This is how a
# stock Rocky Linux 8 box, whose bison is too old, can still build the tool.
PREBUILT := $(shell test -f .prebuilt && echo 1 || echo 0)
ifeq ($(PREBUILT),1)
GEN_DIR   := gen
else
GEN_DIR   := $(BUILD_DIR)/gen
endif

# Two programs, built from the same front end:
#   icl-format  the command line formatter
#   icl-lsp     a Language Server (diagnostics + formatting) for editors
# Each is produced both dynamically linked and fully static.
BIN            := $(BUILD_DIR)/$(PACKAGE)
LSP_BIN        := $(BUILD_DIR)/icl-lsp
STATIC_BIN     := $(BUILD_DIR)/$(PACKAGE)-static
LSP_STATIC_BIN := $(BUILD_DIR)/icl-lsp-static

CXXFLAGS  ?= -O2
CXXSTD    := -std=c++17
WARNINGS  := -Wall -Wextra -Wno-unused-parameter
CPPFLAGS  += -I$(SRC_DIR) -I$(GEN_DIR) -D$(shell echo $(PACKAGE) | tr a-z- A-Z_)_VERSION='"$(VERSION)"'

# Hand-written sources.
#
# The parser front end is shared: icl-lsp links the same objects as
# icl-format, so a grammar fix reaches both and neither can drift.
SHARED_SOURCES := \
	$(SRC_DIR)/icl_driver.cc \
	$(SRC_DIR)/format_tree.cc

CLI_SOURCES := $(SRC_DIR)/icl_format_main.cc
LSP_SOURCES := $(SRC_DIR)/json.cc $(SRC_DIR)/icl_lsp_main.cc

SOURCES := $(SHARED_SOURCES) $(CLI_SOURCES) $(LSP_SOURCES)

# bison / flex outputs. Kept in $(GEN_DIR) so that `make clean` cannot leave a
# stale generated parser behind a hand-edited .yy file.
PARSER_CC  := $(GEN_DIR)/icl_parser.cc
PARSER_HH  := $(GEN_DIR)/icl_parser.hh
LEXER_CC   := $(GEN_DIR)/icl_lexer.cc
LEXER_HH   := $(GEN_DIR)/icl_lexer.hh

GEN_SOURCES := $(PARSER_CC) $(LEXER_CC)

SHARED_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SHARED_SOURCES)) \
                  $(patsubst $(GEN_DIR)/%.cc,$(BUILD_DIR)/%.o,$(GEN_SOURCES))
CLI_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(CLI_SOURCES))
LSP_OBJECTS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(LSP_SOURCES))

OBJECTS := $(SHARED_OBJECTS) $(CLI_OBJECTS) $(LSP_OBJECTS)

# ---- statically linked binary ----
#
# `-static` folds libstdc++/libgcc/libc into the executable, so the result runs
# on any Linux with a compatible kernel and needs no runtime libraries at all.
# Costs ~2.1 MB (stripped) instead of ~146 KB; pays off when the binary has to
# be dropped onto machines whose glibc differs from the build host's.
#
# ponytail: static glibc is safe here only because the tool uses no NSS, no
# dlopen and no locale-dependent APIs. Upgrade path: if any of those appear,
# drop `-static` (or switch to musl) rather than shipping a subtly broken
# binary.
.PHONY: all dynamic static install uninstall check nvimcheck coccheck clean distclean version dist check-tools
.DEFAULT_GOAL := all

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
#     that only g++ is needed.
BISON_MAJOR := $(shell $(BISON) --version 2>/dev/null | head -1 | sed -E 's/.* ([0-9]+)\.([0-9]+).*/\1/')
BISON_MINOR := $(shell $(BISON) --version 2>/dev/null | head -1 | sed -E 's/.* ([0-9]+)\.([0-9]+).*/\2/')

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

all: check-tools $(BIN) $(LSP_BIN) $(STATIC_BIN) $(LSP_STATIC_BIN)

# For a box without the static C runtime installed (glibc-static / libstdc++
# -static): build the two dynamic binaries only.
dynamic: check-tools $(BIN) $(LSP_BIN)

$(BIN): $(SHARED_OBJECTS) $(CLI_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

$(LSP_BIN): $(SHARED_OBJECTS) $(LSP_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

static: check-tools $(STATIC_BIN) $(LSP_STATIC_BIN)

$(STATIC_BIN): $(SHARED_OBJECTS) $(CLI_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ -o $@

$(LSP_STATIC_BIN): $(SHARED_OBJECTS) $(LSP_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -static $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(PARSER_HH) | $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(GEN_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# ---- generated sources ----
ifeq ($(PREBUILT),1)
# Sources already generated; nothing to do.
$(PARSER_CC) $(PARSER_HH) $(LEXER_CC) $(LEXER_HH):
	@test -f $@ || { echo "error: missing prebuilt $@" >&2; exit 1; }
else
# --report=all writes $(GEN_DIR)/icl_parser.output, which is where shift/reduce
# and reduce/reduce conflicts show up. A conflict is a grammar bug, so fail the
# build instead of silently letting bison pick a resolution.
$(PARSER_CC) $(PARSER_HH): $(SRC_DIR)/icl_parser.yy
	@mkdir -p $(GEN_DIR)
	$(BISON) --report=all --output=$(PARSER_CC) --defines=$(PARSER_HH) $<
	@if grep -q "conflict" $(GEN_DIR)/icl_parser.output 2>/dev/null; then \
		echo "error: grammar has conflicts; see $(GEN_DIR)/icl_parser.output" >&2; \
		exit 1; \
	fi

$(LEXER_CC) $(LEXER_HH): $(SRC_DIR)/icl_lexer.l $(PARSER_HH)
	@mkdir -p $(GEN_DIR)
	$(FLEX) --outfile=$(LEXER_CC) --header-file=$(LEXER_HH) $<
endif

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---- installation ----
#
# `make install` writes a manifest so that `make uninstall` removes exactly the
# files that were installed — no guessing, and no deleting anything that was
# already in the target directory.
MANIFEST := $(BUILD_DIR)/install_manifest.txt

install: all
	@mkdir -p $(bindir) $(includedir)/$(PACKAGE) $(docdir) $(man1dir)
	$(INSTALL_PROGRAM) $(BIN) $(bindir)/$(PACKAGE)
	$(INSTALL_PROGRAM) $(STATIC_BIN) $(bindir)/$(PACKAGE)-static
	$(INSTALL_PROGRAM) $(LSP_BIN) $(bindir)/icl-lsp
	$(INSTALL_PROGRAM) $(LSP_STATIC_BIN) $(bindir)/icl-lsp-static
	$(INSTALL_DATA) $(SRC_DIR)/icl_ast.h $(includedir)/$(PACKAGE)/icl_ast.h
	$(INSTALL_DATA) $(SRC_DIR)/icl_driver.h $(includedir)/$(PACKAGE)/icl_driver.h
	$(INSTALL_DATA) $(SRC_DIR)/format_tree.h $(includedir)/$(PACKAGE)/format_tree.h
	$(INSTALL_DATA) $(SRC_DIR)/json.h $(includedir)/$(PACKAGE)/json.h
	@if [ -f $(PACKAGE).1 ]; then $(INSTALL_DATA) $(PACKAGE).1 $(man1dir)/; fi
	@: > $(MANIFEST)
	@echo $(bindir)/$(PACKAGE) >> $(MANIFEST)
	@echo $(bindir)/$(PACKAGE)-static >> $(MANIFEST)
	@echo $(bindir)/icl-lsp >> $(MANIFEST)
	@echo $(bindir)/icl-lsp-static >> $(MANIFEST)
	@echo $(includedir)/$(PACKAGE)/icl_ast.h >> $(MANIFEST)
	@echo $(includedir)/$(PACKAGE)/icl_driver.h >> $(MANIFEST)
	@echo $(includedir)/$(PACKAGE)/format_tree.h >> $(MANIFEST)
	@echo $(includedir)/$(PACKAGE)/json.h >> $(MANIFEST)
	@if [ -f $(PACKAGE).1 ]; then echo $(man1dir)/$(PACKAGE).1 >> $(MANIFEST); fi
	@echo "installed $(PACKAGE) + icl-lsp $(VERSION) into $(prefix)"

uninstall:
	@if [ ! -f $(MANIFEST) ]; then \
		echo "no manifest at $(MANIFEST); removing known targets instead" >&2; \
		rm -f $(bindir)/$(PACKAGE); \
		rm -f $(bindir)/$(PACKAGE)-static; \
		rm -f $(bindir)/icl-lsp; \
		rm -f $(bindir)/icl-lsp-static; \
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
	@-rmdir $(includedir)/$(PACKAGE) $(docdir) 2>/dev/null || true
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
# needs vim 9 (vim 8.2 rejects its vim9script), `setfiletype` loses to vim's
# built-in `*.icl -> clean` mapping, and the editor's shiftwidth is what
# decides indent width. Skipped when the pieces are absent.
coccheck: $(LSP_BIN)
	@if [ ! -d "$(COC_PLUGIN)" ]; then \
		echo "coc.nvim not prepared; run tests/setup_coc.sh first"; \
	else \
		COC_ROOT=$(COC_ROOT) COC_PLUGIN=$(COC_PLUGIN) COC_VIM=$(COC_VIM) \
			tests/run_coc_check.sh; \
	fi

check: all unitcheck
	@python3 tests/run_tests.py --binary $(BIN)
	@python3 tests/lsp_test.py --binary $(LSP_BIN) --cli $(BIN)

COC_ROOT   ?= /tmp/cocvim
COC_PLUGIN ?= $(COC_ROOT)/coc.nvim
COC_VIM    ?= /usr/local/vim9/bin/vim

# ---- unit tests ----
#
# The AST and the layout pass take plain line numbers (not Bison's location
# type), so they can be tested without generating a parser. That independence
# is deliberate: it is what makes the two passes separately verifiable.
UNIT_BIN := $(BUILD_DIR)/unit_tests

unitcheck: $(UNIT_BIN)
	@$(UNIT_BIN)

$(UNIT_BIN): tests/unit_tests.cc $(SRC_DIR)/format_tree.cc $(SRC_DIR)/icl_ast.h $(SRC_DIR)/format_tree.h
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) \
		tests/unit_tests.cc $(SRC_DIR)/format_tree.cc -o $@

# ---- source distribution with the parser pre-generated ----
#
# Lets a machine without bison/flex (e.g. stock Rocky Linux 8, which ships
# bison 3.0.4) build with nothing but g++. The tarball contains the generated
# parser and scanner sources plus a stub Makefile that skips the generators.
DIST_DIR := $(PACKAGE)-$(VERSION)

dist: all
	@rm -rf $(DIST_DIR)
	@mkdir -p $(DIST_DIR)/src $(DIST_DIR)/samples $(DIST_DIR)/tests $(DIST_DIR)/gen
	cp $(SOURCES) $(DIST_DIR)/src/
	cp $(SRC_DIR)/*.h $(DIST_DIR)/src/
	cp $(SRC_DIR)/icl_parser.yy $(SRC_DIR)/icl_lexer.l $(DIST_DIR)/src/
	cp $(PARSER_CC) $(PARSER_HH) $(LEXER_CC) $(LEXER_HH) $(DIST_DIR)/gen/
	cp $(GEN_DIR)/location.hh $(DIST_DIR)/gen/ 2>/dev/null || true
	cp samples/*.icl $(DIST_DIR)/samples/ 2>/dev/null || true
	cp tests/*.py tests/*.vim tests/*.sh tests/*.lua $(DIST_DIR)/tests/
	cp Makefile $(DIST_DIR)/Makefile
	@echo "PREBUILT = 1" > $(DIST_DIR)/.prebuilt
	tar czf $(DIST_DIR).tar.gz $(DIST_DIR)
	@rm -rf $(DIST_DIR)
	@echo "created $(DIST_DIR).tar.gz (build with only g++: no bison/flex needed)"

version:
	@echo $(VERSION)

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(GEN_DIR)
