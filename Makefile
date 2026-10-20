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
#
# format_job.cc is the shared *job* layer: everything between "an indent width
# was requested" and "here is the formatted text". Both binaries link it, so
# the two front ends cannot disagree about style clamping or error phrasing.
SHARED_SOURCES := \
	$(SRC_DIR)/icl_driver.cc \
	$(SRC_DIR)/format_tree.cc \
	$(SRC_DIR)/format_job.cc

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
.PHONY: all dynamic static install install-dynamic install-static uninstall check nvimcheck coccheck clean distclean version dist check-tools
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

# -MMD -MP: write a .d file listing the headers each object depends on, and
# emit a phony target for every one of them. Without this, editing a header
# leaves stale objects in place — which is not a subtle bug: changing a
# private member of Json and running `make` produced a binary that segfaulted
# on its first message, because icl_lsp_main.o still used the old layout.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(PARSER_HH) | $(BUILD_DIR)
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

$(PARSER_CC) $(PARSER_HH): $(SRC_DIR)/icl_parser.yy
	@mkdir -p $(GEN_DIR)
	$(BISON) $(BISON_FLAGS) --output=$(PARSER_CC) --defines=$(PARSER_HH) $<
	@if grep -q "conflict" $(GEN_DIR)/icl_parser.output 2>/dev/null; then \
		echo "error: grammar has conflicts; see $(GEN_DIR)/icl_parser.output" >&2; \
		exit 1; \
	fi

# flex reports "rule cannot be matched" — a scanner rule shadowed by an
# earlier one, so it can never fire. Silent today, and invisible tomorrow.
$(LEXER_CC) $(LEXER_HH): $(SRC_DIR)/icl_lexer.l $(PARSER_HH)
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
# `make codegraph` refreshes docs/codegraph/. Both inputs are external tools;
# the point of the target is that the committed artefacts can be reproduced.
#
#   * tests_codegraph.html / .csv — `codegraph` over the Python test helpers.
#     codegraph (1.2.0) parses with Python's own tokenizer, so it only sees
#     .py files; it reports nothing for the C++ sources.
#   * cpp_deps.* — the C++ half, taken from the compiler: `g++ -MM` already
#     knows every header each translation unit includes, so there is no
#     second parser to keep in sync.
codegraph: $(ALL_OBJECTS)
	@mkdir -p docs/codegraph
	@command -v codegraph >/dev/null 2>&1 || { \
		echo "error: codegraph not installed (pip install codegraph)" >&2; exit 1; }
	codegraph tests --output docs/codegraph/tests_codegraph.html \
		--csv docs/codegraph/tests_codegraph.csv
	@for f in $(SRC_DIR)/*.cc; do \
		$(CXX) $(CXXSTD) -I$(SRC_DIR) -I$(GEN_DIR) -MM $$f; \
	done > docs/codegraph/cpp_deps.makefile
	@python3 tests/deps_to_dot.py docs/codegraph
	@echo "codegraph: docs/codegraph/ refreshed"

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
COV_CXXFLAGS := -fprofile-arcs -ftest-coverage -g -O0

# One set of instrumented objects, linked three ways. Each binary has its own
# `main`, so the other two mains are filtered out rather than listed by hand.
COV_SHARED := $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(SHARED_SOURCES)) \
              $(patsubst $(GEN_DIR)/%.cc,$(COV_DIR)/%.o,$(GEN_SOURCES))
COV_CLI := $(COV_SHARED) $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(CLI_SOURCES))
COV_LSP := $(COV_SHARED) $(patsubst $(SRC_DIR)/%.cc,$(COV_DIR)/%.o,$(LSP_SOURCES))
COV_UNIT := $(COV_SHARED) $(COV_DIR)/json.o $(COV_DIR)/unit_tests.o
COV_BINARIES := $(COV_DIR)/icl-format $(COV_DIR)/icl-lsp $(COV_DIR)/unittests
COV_REPORT_OBJECTS := $(COV_CLI) $(COV_DIR)/json.o \
                       $(COV_DIR)/icl_lsp_main.o $(COV_DIR)/unit_tests.o

$(COV_DIR):
	mkdir -p $(COV_DIR)

$(COV_DIR)/%.o: $(SRC_DIR)/%.cc $(PARSER_HH) | $(COV_DIR)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/%.o: $(GEN_DIR)/%.cc | $(COV_DIR)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/unit_tests.o: tests/unit_tests.cc $(PARSER_HH) | $(COV_DIR)
	$(CXX) $(CXXSTD) $(COV_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(COV_DIR)/icl-format: $(COV_CLI)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

$(COV_DIR)/icl-lsp: $(COV_LSP)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

$(COV_DIR)/unittests: $(COV_UNIT)
	$(CXX) $(CXXSTD) --coverage $^ -o $@

coverage: $(COV_BINARIES)
	@rm -f $(COV_DIR)/*.gcda *.gcov
	@python3 tests/regression.py --binary $(COV_DIR)/icl-format
	@python3 tests/run_tests.py --binary $(COV_DIR)/icl-format
	@python3 tests/lsp_test.py --binary $(COV_DIR)/icl-lsp \
		--cli $(COV_DIR)/icl-format
	@$(COV_DIR)/unittests
	@gcov -b -c -o $(COV_DIR) $(COV_REPORT_OBJECTS) > /dev/null
	@python3 tests/coverage.py --min-line 100 --min-branch 90 --strict

# ---- grammar and scanner coverage ----
#
# Measures which *lines of the hand-written files* the corpus exercises:
# src/icl_parser.yy and src/icl_lexer.l. The generated C++ is not measured —
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

$(GRAMCOV_DIR):
	mkdir -p $(GRAMCOV_DIR)

$(GRAMCOV_PARSER): $(SRC_DIR)/icl_parser.yy | $(GRAMCOV_DIR)
	$(BISON) -t --output=$(GRAMCOV_PARSER) --defines=$(GRAMCOV_DIR)/icl_parser.hh $<

# bison writes the header alongside the .cc, so it has no recipe of its own;
# an empty one keeps make from concluding the target is impossible.
$(GRAMCOV_DIR)/icl_parser.hh: $(GRAMCOV_PARSER) ;
$(GRAMCOV_DIR)/icl_lexer.h: $(GRAMCOV_LEXER) ;

$(GRAMCOV_LEXER): $(SRC_DIR)/icl_lexer.l $(GRAMCOV_DIR)/icl_parser.hh
	$(FLEX) -d --outfile=$(GRAMCOV_LEXER) --header-file=$(GRAMCOV_DIR)/icl_lexer.h $<

$(GRAMCOV_DIR)/%.o: $(SRC_DIR)/%.cc $(GRAMCOV_DIR)/icl_parser.hh | $(GRAMCOV_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
		$(CPPFLAGS) -c $< -o $@

$(GRAMCOV_DIR)/icl_parser.o: $(GRAMCOV_PARSER) | $(GRAMCOV_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
		$(CPPFLAGS) -c $< -o $@

$(GRAMCOV_DIR)/icl_lexer.o: $(GRAMCOV_LEXER) | $(GRAMCOV_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -DICL_PARSER_TRACE -I$(GRAMCOV_DIR) \
		$(CPPFLAGS) -c $< -o $@

$(GRAMCOV_BIN): $(GRAMCOV_OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $^ -o $@

gramcov: $(GRAMCOV_BIN)
	@python3 tests/grammar_coverage.py --binary $(GRAMCOV_BIN) \
		--parser $(GRAMCOV_PARSER) --lexer $(GRAMCOV_LEXER)

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
#   install-static   self-contained; runs on any Linux regardless of glibc
#
# Because both write the same paths, the manifest is correct whichever ran.
HEADERS := $(SRC_DIR)/icl_ast.h $(SRC_DIR)/icl_driver.h \
           $(SRC_DIR)/format_tree.h $(SRC_DIR)/json.h

install: install-dynamic

# One recipe body, parameterised by (formatter, lsp, label). The two targets
# differ only in which files they install — everything else, manifest included,
# is identical, so it is written once.
define install_body
	@mkdir -p $(bindir) $(includedir)/$(PACKAGE) $(docdir) $(man1dir)
	$(INSTALL_PROGRAM) $1 $(bindir)/$(PACKAGE)
	$(INSTALL_PROGRAM) $2 $(bindir)/icl-lsp
	@for h in $(HEADERS); do \
		$(INSTALL_DATA) $$h $(includedir)/$(PACKAGE)/; \
	done
	@if [ -f $(PACKAGE).1 ]; then $(INSTALL_DATA) $(PACKAGE).1 $(man1dir)/; fi
	@: > $(MANIFEST)
	@echo $(bindir)/$(PACKAGE) >> $(MANIFEST)
	@echo $(bindir)/icl-lsp >> $(MANIFEST)
	@for h in $(HEADERS); do \
		echo $(includedir)/$(PACKAGE)/`basename $$h` >> $(MANIFEST); \
	done
	@if [ -f $(PACKAGE).1 ]; then echo $(man1dir)/$(PACKAGE).1 >> $(MANIFEST); fi
	@echo "installed ($3) $(PACKAGE) + icl-lsp $(VERSION) into $(prefix)"
	@$(call coc_body,$3)
endef

# Printed after every install: the editor config is the one thing a user has to
# write by hand, and it differs between the two programs, so guessing it is
# where people get stuck. Both snippets are emitted together because a real
# setup uses icl-lsp for live diagnostics and icl-format for batch runs.
#
# `set filetype=icl` — not `setfiletype` — is deliberate: vim's own filetype.vim
# maps *.icl to the Clean language, and setfiletype silently does nothing once
# a filetype is already set, which leaves coc never starting the server.
define coc_body
	@echo ""
	@echo "=== coc.nvim configuration ==="
	@echo ""
	@echo "1) Add to coc-settings.json (open it with :CocConfig):"
	@echo ""
	@echo '  {'
	@echo '    "languageserver": {'
	@echo '      "icl": {'
	@echo '        "command": "icl-lsp",'
	@echo '        "filetypes": ["icl"],'
	@echo '        "settings": {}'
	@echo '      }'
	@echo '    },'
	@echo '    "coc.preferences.formatOnSave": true'
	@echo '  }'
	@echo ""
	@echo "2) Add to vimrc — both lines matter:"
	@echo ""
	@echo '  autocmd BufNewFile,BufRead *.icl set filetype=icl'
	@echo '  autocmd FileType icl setlocal shiftwidth=2 tabstop=2 expandtab'
	@echo ""
	@echo "   coc.nvim needs vim 9 — its api.vim is vim9script. vim 9.2 is what"
	@echo "   tests/setup_coc.sh builds (VIM_VERSION=v9.2.0000); vim 8.2 fails"
	@echo "   to start the server without saying why."
	@echo ""
	@echo "   shiftwidth is what coc forwards as LSP tabSize, so the editor and"
	@echo "   the CLI agree only if it is 2 — vim defaults to 8."
	@echo ""
	@echo "3) icl-lsp is now active. Check with :CocCommand workspace.showOutput;"
	@echo "   formatting is :call CocAction('format')."
	@echo ""
	@echo "4) icl-format is the batch tool — same grammar, no editor needed:"
	@echo ""
	@echo '    icl-format file.icl              # print formatted'
	@echo '    icl-format -w file.icl           # rewrite in place'
	@echo '    icl-format --check file.icl      # exit 1 if it would change'
	@echo '    icl-format --ast file.icl        # show the syntax tree'
	@echo ""
	@echo "   To run icl-format from vim on the current buffer instead:"
	@echo ""
	@echo '  autocmd FileType icl nnoremap <buffer> <leader>f \'
	@echo '        :%!icl-format<CR>'
	@echo ""
	@echo "Installed variant: $1; binaries are at $(bindir)/$(PACKAGE)"
	@echo "and $(bindir)/icl-lsp."
endef

install-dynamic: $(BIN) $(LSP_BIN)
	$(call install_body,$(BIN),$(LSP_BIN),dynamic)
	@echo "for a glibc-independent build: make install-static"

install-static: $(STATIC_BIN) $(LSP_STATIC_BIN)
	$(call install_body,$(STATIC_BIN),$(LSP_STATIC_BIN),static)

uninstall:
	@if [ ! -f $(MANIFEST) ]; then \
		echo "no manifest at $(MANIFEST); removing known targets instead" >&2; \
		rm -f $(bindir)/$(PACKAGE); \
		rm -f $(bindir)/icl-lsp; \
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
# needs vim 9.2 (vim 8.2 rejects its vim9script), `setfiletype` loses to vim's
# built-in `*.icl -> clean` mapping, and the editor's shiftwidth is what
# decides indent width. Skipped when the pieces are absent.
coccheck: $(LSP_BIN)
	@if [ ! -d "$(COC_PLUGIN)" ]; then \
		echo "coc.nvim not prepared; run tests/setup_coc.sh first"; \
	else \
		COC_ROOT=$(COC_ROOT) COC_PLUGIN=$(COC_PLUGIN) COC_VIM=$(COC_VIM) \
			tests/run_coc_check.sh; \
	fi

# The corpus in tests/cases/ runs first: it is the cheapest way to see a
# regression, and a failure there names the file that broke.
check: all unitcheck
	@python3 tests/regression.py --binary $(BIN)
	@python3 tests/golden_test.py --binary $(BIN)
	@python3 tests/fuzz.py --binary $(BIN)
	@python3 tests/block_review.py --binary $(BIN)
	@python3 tests/run_tests.py --binary $(BIN)
	@python3 tests/lsp_test.py --binary $(LSP_BIN) --cli $(BIN)
	@$(MAKE) --no-print-directory gramcov

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

$(UNIT_BIN): tests/unit_tests.cc $(SRC_DIR)/format_tree.cc \
             $(SRC_DIR)/json.cc $(SRC_DIR)/icl_ast.h $(SRC_DIR)/format_tree.h \
             $(SRC_DIR)/json.h
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) \
		tests/unit_tests.cc $(SRC_DIR)/format_tree.cc $(SRC_DIR)/json.cc -o $@

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
	# Recursive: the corpus is sorted into per-source subdirectories, and a
	# flat copy would ship an empty set while appearing to succeed.
	cp -r tests/cases tests/golden $(DIST_DIR)/tests/
	# unit_tests.cc is part of `make check` (the `unitcheck` target builds it),
	# so a tarball without it cannot run its own test suite. The copy list was
	# once only .py/.vim/.sh/.lua, and `make dist` still reported success.
	cp tests/*.py tests/*.cc tests/*.vim tests/*.sh tests/*.lua $(DIST_DIR)/tests/
	cp Makefile README.md REQUIREMENT.md $(DIST_DIR)/
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
