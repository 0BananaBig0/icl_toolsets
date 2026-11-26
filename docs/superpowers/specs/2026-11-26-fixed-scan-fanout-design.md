# Fixed fanout scan networks

Extend completion-program item 2 from equal disjoint shared-input lanes to
interface-free fixed fanout: unequal lane lengths, repeated full-register
storage, and zero-storage wire observations alongside a nonempty lane. This
lets the existing compiler retarget one physical input without inventing
independent values for its observations. Parent: master at 519b992e.

## Approach

Reuse the current graph, symbolic register loads and concrete capture/shift/update.
Every root still has one unambiguous ultimate input. Every group sharing that
input must have fixed, ungated routes. Concatenate each path's corrected load
words input-most first. Each shorter nonempty word must equal the most significant
prefix of the longest word, conditional on a scan event. An empty observation
adds no load constraint. Replay each physical input's longest chronological
stream on every corresponding lane before global padding. Preserve output
comparisons and clock each physical register once.

Merely removing guards admits impossible loads. A new per-clock symbolic engine
adds work unnecessary for fixed routes. The existing word model is the selected
implementation, an engineering choice under the user's delegated completion and
native-execution instructions.

## Bounds and contracts

Preserve scoped controller permissions from the preceding shared-input repair,
omitted loads, actual capture histories, logical read masks, once-only status,
original/flat/generated replay, reservations, copying, rollback and output safety.
Keep intermediate register taps, switched/gated sharing and named/client/TAP
sharing guarded. Existing equal disjoint groups retain their current behavior.

New geometries require resets invariant and inactive throughout CSU. Admit only
constant/global resets or direct ResetPort forwarding ending at a root ResetPort;
check actual inactivity during replay. Reject other reset dependencies rather
than infer stability from endpoints. General serial-derived/local reset scheduling
remains item 3. No new dependencies, resource limits or hardware/timing claims.

## Acceptance

TDD compatible shared-storage (A=13,B=5), unequal (A=13,B=3), forked
(A=13,B=2,C=10), wire observations and incompatible loads. Literal independent
shift oracle for forked storage: input 01011011, eight ticks, final 13/2/10.
Check inversion, ascending indices, hierarchy, partial reads/writes, actual
capture 0101 rather than goal 1101, status clearing and failed publication.
Check inactive root resets and retained reset/topology boundaries. Maintain native
scheduler coverage and the normal/static CLI regression. Measure 2/8/32 fanout
observations at 4/64/256 bits, four replay forms under 10 seconds and 512 MiB;
record equivalent parent/current controls. Full make check, affected sanitizers,
C++ quality and packaging precede one fresh final review. Merge into master,
verify, push and delete only this owned task branch after the gates pass.
