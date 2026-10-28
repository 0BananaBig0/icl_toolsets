# Data-register execution implementation plan

> **For agentic workers:** Use superpowers:executing-plans, TDD, independent review, and full regression.

**Goal:** Extend the shared event model and retargeter to selectable and addressed DataRegisters.

**Architecture:** Reuse AddressResolver for address domains and enables, NetworkState for register storage, and SymbolicNetwork for value justification. Evaluate cascaded data registers in dependency order after update/parallel input events; retain unknowns and reset priority. Use bounded deterministic batching rather than schedule enumeration.

**Tech Stack:** Existing C++17/Z3 frontends, state, solver, Python/native regression, Tessent directional harness.

**Spec:** IEEE 1687-2014 6.4.9 (printed pp.117–121), 6.5.2(h), user's retargeter requirements and docs/pattern-generation.md. Tessent v2023.4 DataRegister topic supplements default connections.

## Global constraints

- Continue authorized checkout/master; reuse all shared AST/netlist interfaces.
- Keep atomic output/state publication, unknown propagation and explicit reset semantics.
- New test scripts beneath tests/src/pattern; fixture provenance under tests/cases.
- Preserve existing budgets; cache structural dependencies rather than enumerate patterns.
- Callback execution, multiple controllers and system clocks remain separate requirements.

## Review focus

- Cascades, reconvergent controls and feedback; reset precedence and uncertain enables.
- Address offsets, direction/width, independent selection and aliased targets.
- No partial state mutation on unsupported connections or resource errors.
- Defaults, reservations, repeated writes and generated-PDL idempotency.
- Shared buses with simultaneous versus sequential transactions.

### Task 1: Shared data-register events

- [x] Add native tests for selectable cascades, held/unknown enables, addressed loads and reset interactions; observe failure before implementation.
- [x] Cache data-register activation/dependency models; use dependency-ordered event evaluation in NetworkState.
- [x] Add atomic parallel-input update entry point and include data updates after qualified scan updates.
- [x] Verify native state/TAP/symbolic suites and bounded larger cascades.

### Task 2: Retargeted access and observable outputs

- [x] Add failing CLI tests for selectable/addressed writes, aliases/defaults, shared-bus batching and round trips.
- [x] Reuse primary-input justification; resolve address/select/data requirements with concrete replay. Concrete scan events share the model, but scan-driven planning remains unfinished.
- [x] Extend addressed OneHot observation and data-register read routing with native/CLI tests.
- [x] Qualify supported paths using existing Tessent harness; preserve failed comparisons.

### Task 3: Publication

- [x] Independent review, focused checks, complete make check, Google style and leak-enabled sanitizers.
- [x] Update current capabilities/requirements and evidence in README/docs, commit and push.

## Publication boundaries

This phase completes primary-input DataRegister access and shared concrete events.
It does not complete the full user specification. Serial/mixed DataRegister
planning, callbacks, feedback updates, OneHotScanGroup, multiple-controller/output
scheduling, independent/shadow events and system clocks remain tracked in
`docs/pattern-generation.md`. Nested address-default vendor equivalence is still
unqualified; the raw mismatches and reasons are in `docs/tessent-goldens.md`.

Independent review found three important failures (uncertain side effects,
destructive output reads and end-state marker replay); all have RED→GREEN
regressions. The publication checks and measured scope are recorded in the
DataRegister execution verification section of the implementation documentation.
