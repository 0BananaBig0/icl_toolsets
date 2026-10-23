# PDL block review implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline under the user's standing autonomous execution authorization. Preserve the current uncommitted PDL work.

**Goal:** Review IEEE1687-2014 PDL in normative block order, repair statically decidable syntax gaps, and make each reviewed block reproducibly testable through both executables.

**Architecture:** Keep the shared pdl::Document command/word tree, analysis and rendering. Reuse Tcl's parsing APIs without evaluation; do not introduce PDL Flex/Bison files or reuse ICL tokens. Add a maintained source-attributed case manifest and executable-boundary runner. Preserve vendor compatibility and unchanged source bytes.

**Tech Stack:** Existing C++17/Tcl, Python standard library, GNU Make.

**Spec:** User's eight-part PDL review request; existing ../specs/2026-10-23-pdl-tools-design.md remains the implementation architecture contract.

## Constraints and review focus

- Enum/DataOutPort are ICL, not PDL blocks. Review PDL lexical/generic rules, numbers, structure and commands instead.
- Finish and run each block before implementing the next. Record clause, current coverage, gaps, fix and actual test result.
- Add positive/negative, boundary and recursive cases under tests/cases by source. Put new scripts under tests/src; keep generated products ignored.
- Non-evaluating static editor tools do not simulate retargeting, hardware state or arbitrary Tcl dataflow. State runtime-only obligations explicitly.
- Use CodeGraph for structural impact; current source and reproducible results decide correctness.
- Retain all79vendor files and6narrow expected diagnostic cases. Validate dynamic unknown contexts without inventing targets.
- Preserve source bytes/string contents and nesting limits; unknown Tcl commands stay opaque.
- Observe RED→GREEN for each behavior fix. Run focused block cases, then full dynamic/static checks and strict unchanged coverage. No commit/push while gates fail.
- Final independent review checks literal versus runtime values, vector/substitution distinctions, Tcl body/data boundaries, formatter byte preservation and negative-case specificity.

## Sequential tasks

1. **Lexical/generic tokens and numbers (§§7.5.3,7.6–7.8).** Inspect standard and existing cases; add maintained block manifest/runner with numeric, identifier, comment, continuation and vector cases. Observe gaps before editing shared validators; run only this block to confirmation.
2. **Procedure structure and binding (§§7.9.1–8).** Add namespaced module/procedure/prefix, argument/default/order and nested Tcl cases. Cover keywords as names and unresolved contexts; fix shared syntax checks and confirm this block.
3. **Data/scan commands (§§7.9.9–13).** Verify iReset/iRead/iWrite/iScan/iOverrideScanInterface individually, each with options/negative/boundary cases and normative examples. Confirm each before the next.
4. **Apply/clock/run/merge/state (§§7.9.14–22).** Check each command's precise forms and examples; add missing time/frequency/option validation and source-attributed tests. Keep documented extensions separate.
5. **Level1 and recursion (§§8.1–8.5).** Check readback/status/failure forms, nested control/substitution cases, depth and incomplete-buffer diagnostics; cover authored branches where feasible.
6. **Upstream comparison, quality, verification and installation.** Compare Honza255's separate pdl.g4 and tests at pinned revision against IEEE; record missing cases/keywords and upstream discrepancies. Audit shared architecture/dead work, wire pdlcheck and coverage, run style/full/static/coverage checks and one independent review. Install/verify both variants and print pdl-lsp formatting configuration for coc.nvim.

Implementation files: src/pdl_document.{h,cc}; tests/src/test_pdl_blocks.py;
tests/cases/pdl_blocks.json and categorized PDL block fixtures; Makefile;
docs/pdl-block-review.md, docs/pdl-tools.md, docs/requirements.md and source attribution.
