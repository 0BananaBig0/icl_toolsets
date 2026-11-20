# Current completion records and reference verification
Implement requested items 14 and 15 as one evidence/documentation increment.
Use the exact Task 137 snapshot400d2ba evidence for current product descriptions.
Replace stale current summaries rather than rewriting dated historical evidence.
Keep historical checklists, explicitly point them to the reconciled current
tracker, and preserve every outstanding requirement, including excluded item 1.
The tracker must retain all items 2–15 until concrete implementation and
verification completes each. Do not turn exclusions into completed features.

Run all six existing ANTLR adapter tests, including the five formerly skipped
reference cases, with Honza255/icl_parser pinned at
333f8bc7a54964138ac14a34fde9e942c496436d and
antlr4-python3-runtime 4.7.2 in owned temporary directories. No application
dependency, parser implementation or machine-global installation changes.
Record exact commands, versions and status. Verify current doc numbers and
links against the actual completion artifact, inspect the final diff and get
one independent whole-branch review. Preserve runtime input/product identity.
