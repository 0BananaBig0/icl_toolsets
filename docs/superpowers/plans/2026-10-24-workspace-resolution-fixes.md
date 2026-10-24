# Workspace and PDL resolution fixes

> Execute inline with superpowers:executing-plans; obtain an independent review
> after implementation. The user's request approves the six reviewed fixes.

**Goal:** Correct workspace identity/exclusions and Tcl navigation, and remove
quadratic PDL-to-ICL reference lookup.

**Architecture:** Keep the shared LSP transport and separate language documents.
Use canonical filesystem identity for automatic indexing and open overlays,
retaining client URIs. Reuse PDL scope/binding structures and add a per-module
ICL-member index in the PDL workspace. No grammar/token sharing or dependencies.

**Spec:** The preceding review's six reproduced defects and the user's request
to fix them and run complete regression. Continue in this authorized checkout.

## Constraints and review focus

- Keep explicit opens in excluded directories usable; watcher events must not
  import automatically excluded copies or overwrite dirty buffers via aliases.
- Preserve client URIs, alias closes, deletes, retargeted symlinks and genuine
  duplicate declarations. Never treat different files as the same module.
- Unknown Tcl namespaces must not invent global declarations; explicit absolute
  names and procedure-local variables remain resolvable.
- Static global bindings support qualified names and arrays; do not evaluate PDL.
- Index rebuilds must remove stale members and retain ambiguity rejection.
- New tests stay under tests/; generated logs stay ignored under build/.
- Google source style; no commit/push/install requested in this task.

## Tasks

- [x] Workspace: extend tests/src/test_lsp_server.py with excluded watcher,
  symlink identity and open-overlay lifecycle assertions; run RED. Update
  src/lsp_server.h; run GREEN for both languages.
- [x] Tcl: extend tests/src/test_pdl_tools.py with unknown namespace, relative
  qualified fallback and global binding cases (definitions/references/completion);
  run RED. Update src/pdl_document.{h,cc}; run GREEN.
- [x] Performance: extend tests/src/test_pdl_document.py with ICL object
  references, rebuild/ambiguity checks and a repeatable benchmark. Run RED
  against the existing two-second limit at 32,000 objects. Index members in
  Workspace::Reindex and reuse them in IclObject; rerun equivalently.
- [x] Run independent review, complete dynamic regression, static focused
  checks, format/naming checks and strict authored coverage. Investigate every
  failure, keeping the existing coverage thresholds. Record actual results.

## Baseline evidence

Review probes reproduced all five navigation bugs. PDL ICL-object reference
queries (three runs, Linux WSL2, dynamic binary) took 0.0322–0.0335 s for
4,000 objects, 0.1395–0.1419 s for 8,000, 0.5151–0.5726 s for 16,000 and
2.3678–2.3827 s for 32,000 (1,001,811 combined source bytes). Existing functional
tests pass. Prior strict coverage is 4,141/4,248 lines and 7,002/11,401 branches;
the required 100%/90% gate already fails.

## Execution record

Workspace and Tcl regressions failed before the corresponding production edits.
The 32,000-object performance regression failed at 2.253 seconds before indexing;
three optimized runs measured 4.27–4.48 ms. Final focused suites pass: eight
shared-server and 49 PDL tests, plus the reference scaling assertions.

Independent review found three additional root/lifecycle edge cases; their
reproducing tests and fixes are included. Follow-up review found no further
actionable issues. Static focused suites and Google style checks passed.
See docs/workspace-resolution-review.md for detailed decisions and evidence.

Final complete dynamic regression passed, including both editors' entire
38,488-ICL/728-PDL corpus. Strict coverage's functional suites passed, but its
unchanged 100%/90% gate failed at 4,244/4,355 lines and 7,271/11,833 branches.
That pre-existing acceptance gap remains explicit; this plan does not claim
the overall coverage requirement is fulfilled.
