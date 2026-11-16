# Native ordinary scan replay

Self-constructed, MIT. A visible SIB write/read prepares a raw route, followed
by an ordinary partial leaf write and pre/post-write observations. The native
ordinary implementation must retain the same logical writes, read masks and
physical state as the existing solver. The fixture is small enough for genuine
Tessent bidirectional qualification; million-bit scale and literal-clock proofs
are recorded separately. IEEE1687-2014 ordinary iWrite/iRead/iApply semantics
and the existing mixed root iScan contract apply.

The first forward probe was rejected by strict comparison because Tessent retained
the preceding open1 observation on later applies. The final source explicitly
compares open1 on every later apply, retaining every earlier leaf and SIB check.
The failed probe remains archived; no trace normalization or certificate waiver.
