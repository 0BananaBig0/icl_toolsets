# Clock evidence replay guide (Task210)

Resolve the two Task209 review follow-ups for repository maintainers: give the
narrative erratum an explicit correction date, and explain how to reproduce
the retained measurements and archived-job checks in isolated output storage.

Add the actual local correction date, 2026-11-28 (Asia/Shanghai), to both
matching correction sidecars. Preserve their exact undated bytes before adding
metadata; leave the original/corrected native receipt payloads and canonical
measurement/comparison driver copies unchanged.

Write docs/clock-evidence-reproduction.md for the retained original workspace.
State prerequisites: qualified binaries, parent manifests, archived fixtures,
GNU time and Python. Verify the canonical driver's digest before deriving a
copy. Seed the two qualified manifests in a new owned scratch directory; point
the derived copy there and omit the original metadata-correction tail. Run the
same measurement/vendor-attestation logic and retain original/derived hashes,
commands, outputs and success. Explain that this deliberately adapted copy
replays measurements and archived evidence rather than reapplying an erratum.

Use an active temporary script under repository tests/ and retain its execution
copy beside scratch receipts. Verify canonical evidence hashes before/after.
Do not edit archived drivers or canonical output trees to make another run fit.
No new public producer command or dependency is needed. Keeping the fixed
canonical paths would require destructive cleanup; another maintained replay
framework would duplicate an already archived script. The guide's bounded
derivation is the smallest option matching these artifacts.

Execute the published instructions literally: 45 measurement rows, nine
prepared native inputs, 270 native processes and 16 parser calls with existing
10s/512MiB limits. Preserve eight pairs/16 archived jobs and the native-only
attached-unit control. No fresh vendor/device/timing or speedup claim. Require
an isolated reader to follow only the guide and reader task before technical
review. One whole-branch review serves both phases; no extra reviewer.

Keep all 4135 qualified inputs and 48 products identical. Run source build/install
and exact package-member checks for guide/tracker/spec/plan. Standing opted-out
questions and inline authorization apply. Candidate findings: 27 minus 2 = 25.
Preserve 117 stashes, foreign worktree state and sibling ledgers. Merge, verify,
push, retire only this branch and continue the original inventory.
