# Parser, formatter and LSP test practices

Sources consulted 2026-10-22; upstream behavior is guidance, not an ICL specification.

| Source | Adopted method |
|---|---|
| [Flex debugging options](https://westes.github.io/flex/manual/Debugging-Options.html) | Debug scanner traces identify matched source rules; `%option nodefault` prevents silently echoed unmatched bytes. Trace rule coverage is separate from action branch coverage. |
| [GNU Bison tracing](https://www.gnu.org/software/bison/manual/html_node/Tracing.html) | `%define parse.trace` exposes actual reductions. Task10 counts production IDs rather than only source lines, including alternatives sharing a line. Online manual body was unavailable; indexed official documentation and locally generated trace format were inspected. |
| [Tree-sitter corpus testing](https://tree-sitter.github.io/tree-sitter/creating-parsers/5-writing-tests.html) | Small named cases by grammar block, positive/negative pairs and explicit expected behavior. Our manifest records source and acceptance; AST shape/location assertions are added where consumers depend on them. |
| [clang-format FormatTestBase](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/unittests/Format/FormatTestBase.h) and [FormatTest](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/unittests/Format/FormatTest.cpp) | Golden output, formatting stability, whitespace perturbation, incomplete input and no-crash tests. Our corpus also preserves token/comment/string contents. |
| [clangd protocol.test](https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clangd/test/protocol.test) | Exercise the real executable over framed stdin/stdout, malformed/truncated messages and lifecycle recovery. Task9 extends these tests to semantic replies, Unicode positions and workspace updates; protocol policies follow LSP, not blind copying of clangd behavior. |

Tests execute production binaries. Fixtures live under `tests/cases/`, with IEEE,
Tessent, upstream and constructed origins distinguished. Honza differential results
are adjudicated against source clauses; lexer errors and unconsumed suffixes count
as rejection. Missing external dependencies are unrun (exit77), never a passing test.
