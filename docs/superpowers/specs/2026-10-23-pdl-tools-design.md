# PDL formatter and language server

The user requests dynamic and fully static `pdl-format` and `pdl-lsp`, analogous
to the existing ICL commands, following the discussed Tcl-parser architecture.
Session authorization permits autonomous implementation and repository changes.

Use Tcl's non-evaluating parsing API. Preserve Tcl word spelling, substitutions,
comments and data strings. Recognize script bodies by command semantics rather
than treating every braced word as code. Both PDL executables consume one shared
PDL syntax/semantic tree and formatting implementation. ICL and PDL grammars,
keywords and ASTs remain separate. Never evaluate source to analyze it.

Share the existing CLI file-handling implementation and LSP protocol/server
implementation through language adapters. Retain ICL behavior and regression
coverage. PDL provides diagnostics, formatting, procedure/variable completion,
definition/declaration/implementation navigation, references, hover and symbols.
Resolve statically named procedures with module/procedure namespace context;
dynamic Tcl constructs remain unresolved instead of being executed or guessed.
Support IEEE level declarations and level-0 command restrictions. Recognize PDL
vector indices without misclassifying them as Tcl command invocations.

PDL formatting defaults to two spaces (1–16), follows editor tabSize over LSP,
preserves invalid inputs, supports stdin, -w, -o, --check, --ast, --help/version,
and matches the ICL CLI exit-code convention. Comments/string contents and Tcl
substitution behavior must survive formatting. Limit recursive parsing depth and
avoid quadratic source scans. Validate scripts nested in iProc and standard Tcl
control constructs; retain unknown command arguments conservatively.

Expose dynamic/static and PDL-only build targets. Default builds/installations
include both language pairs; preserve ICL-only targets for environments without
Tcl development files. Dynamic PDL links installed Tcl; fully static PDL requires
a real static Tcl archive, not its stub archive. Keep dependency sources, objects,
generated parsers and binaries outside version control. Include PDL source in
source distributions and print both language-server entries after installation.

Verification includes non-evaluation, nested scripts vs braced data, substitutions,
bracket indices, invalid/incomplete inputs, Unicode diagnostics, CLI parity, real
stdio LSP navigation and corrections, static ELF checks and dynamic/static output
equivalence. Use independently specified fixtures under tests/cases by provenance.
Keep user instructions in README and developer details/limits in docs.

This delivers editor tools, not a Tcl execution environment or IJTAG retargeter.
Runtime-dependent command names, arbitrary eval-generated scripts, electrical
connectivity and complete runtime dataflow cannot be proven by static analysis.
