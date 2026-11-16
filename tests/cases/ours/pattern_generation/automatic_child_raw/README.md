# Automatic child preparation

Self-constructed MIT PDL and parent model reuse RawSibLeaf from gated_sib_raw.
The first child packet starts with its reset-closed parent; setup is generated.
Later comparisons and a partial write check retained child data.

The parent captures unknown data. Tessent adds a reset-value comparison to the
self-capturing parent in the original probe, while this generator requests no
comparison for setup. That strict trace mismatch is retained in build evidence;
the unknown-capture variant permits a strict forward certificate without changing
the comparator. Native/CLI tests separately cover known self-capture and closed
parents. Tessent reverse retargeting rejects the internal child ScanInterface.
