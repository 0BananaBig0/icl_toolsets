# Mixed modeled child raw preparation

Self-constructed, MIT. IEEE1687-2014 §§7.3.3–7.3.4/7.9.12 and Tessent2023.4
iScan describe queued setup followed by ordinary preparation and raw execution.
A leaf-scoped procedure queues local8 before its caller writes the parent SIB
open1. Root output uses the existing full-route CSU. Every pre/post leaf and SIB
comparison is explicit, including later partial-write observations. Genuine
vendor directions are recorded separately; no implicit child opening, opaque
model, TAP/IR, parallel, reset or clock extension is claimed.

Tessent2026.2 forward run/strict record/check passed. Reverse rejected child.local
as an internal ScanInterface, supported only when inferred active for in-system-
test generation in that vendor context. The original source, inputs and console
are archived; no reverse certificate or relaxed comparison is used.
