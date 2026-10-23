# PDL validation follow-up

Continue the approved PDL tools design and prior block review from e86539c.
The user repeats the full review request; preserve completed source comparisons,
corpus provenance and existing checks, and close demonstrated remaining gaps.
Use executing-plans inline with TDD and one final independent read-only review.
Standing autonomous authorization applies; no new approval round is needed.

Architecture remains one non-evaluating pdl::Document AST, analysis and formatter
for both executables. ICL tokens/grammar stay separate. Expected input size is
100 KB–1 MB; avoid repeated whole-source scans and keep the existing depth bound.

1. Lexical block (§§7.5.3, 7.6–7.8): distinguish level-0 argument references
   from arbitrary level-1 substitutions. Add missing malformed/incomplete and
   diagnostic-location cases; confirm before procedure work.
2. Procedures (§§7.9.3–8): inspect mandatory level-0 declaration structure and
   argument lists, context and navigation. Add independently derived boundary
   cases; repair only demonstrated gaps and confirm.
3. Command blocks (§§7.9.9–22 and 8.4): check each existing validator against
   its normative production and missing action branches. Confirm each block
   before moving on, preserving documented vendor compatibility.
4. Nested Tcl (§8): cover known script versus data roles and remaining parser
   error paths. Fix demonstrated false diagnostics or skipped known bodies;
   do not evaluate source or infer arbitrary runtime dataflow.
5. Shared pipeline/quality: cover remaining practical navigation/error paths,
   remove demonstrated redundant code, and check upstream comparison still
   applies to the pinned revision. Request one independent final review.
6. Verify focused/full dynamic and static suites, authored coverage and Google
   style; measure the same representative large file, install both variants,
   update the dated report and print coc.nvim settings. Keep strict coverage
   thresholds unchanged and report unmet requirements accurately.

All new fixtures belong under tests/cases by source; scripts under tests/src.
Use the existing block manifest/runner, adding expected diagnostic message/range
checks where needed. Generated reports/builds stay ignored. No commit/push is
part of this repeated review request.


## Continued review requested after the first follow-up

Reuse the verified 176-block/29-method/79-file baseline and existing reference
extractions; do not redo source collection or introduce a second PDL parser.
Continue inline with the same approved design:

7. Confirm the lexical block, then test procedure argument source positions,
   especially names repeated in an earlier default. Reuse Tcl list spans instead
   of searching later source for the first matching substring.
8. Check literal-only level-0 option/declaration positions against each command
   production, one block at a time. Preserve level-1 runtime values. Add paired
   positive/negative fixtures before fixes.
9. Recheck nested Tcl/data boundaries and coverage gaps, update the upstream
   comparison with exact keyword evidence, and obtain independent review of
   this continuation. Avoid a speculative decoded-source framework unless a
   concrete fixture demonstrates that it is necessary for this change.
10. Run required suites, coverage, static builds and both installations again;
    retain strict thresholds and report remaining limits with coc.nvim settings.
