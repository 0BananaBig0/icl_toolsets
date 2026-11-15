# Mixed root raw preparation

Self-constructed, MIT. Tessent Shell Reference v2023.4 iScan describes ordinary
queued accesses preparing the route before a raw load in one iApply. IEEE1687-2014
§7.9.12 defines raw setup/model data and execution-time length. This fixture writes
the initially closed SIB before a root9-bit raw load, then observes leafA5 and open1.
Explicit pre-write read0, rawSO1 and final leaf/SIB comparisons are retained.
No implicit child preparation or source-order/-together extension is claimed.
