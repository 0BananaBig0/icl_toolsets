# TCK idle between modeled raw scans

A nested static root chain retains controlling fill and register values across
explicit TCK waits. Raw scans and ordinary accesses use the existing native idle
engine. No system-clock, opaque-path or independent-TAP widening.
