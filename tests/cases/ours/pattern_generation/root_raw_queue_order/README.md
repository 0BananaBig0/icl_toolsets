# Root raw queue order

Self-constructed, MIT. A future nine-bit root raw scan is queued before the
ordinary SIB write/read that establishes its route. IEEE1687-2014 §§7.3.3–7.3.4,
7.9.12 and Tessent Shell Reference2023.4 iScan establish queued setup semantics;
the vendor executes raw after ordinary preparation. Every later leaf and SIB
comparison is explicit, including pre/post partial-write observations. No
child preparation or opaque/TAP/parallel timing extension is claimed.
