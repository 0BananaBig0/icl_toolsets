# Port/control/merge qualification cases

These self-constructed IEEE 1687 cases share `network.icl` and the existing
AccessProbe BSDL in `../access_link/simple.bsdl`. They exercise inverted
DataOutPort writes, explicit replacement of a reserved child WriteEnPort,
and two independent read/write procedures inside iMerge.

`cases.json` selects these three cases from the main Tessent golden manifest.
Use the existing directional round-trip harness for preparation/certification.
Remote results are evidence, not checked-in standards until the ordered-apply
comparison succeeds. Ordinary regression uses certified snapshots offline.
