# Indexed scan-input regression cases

These are self-constructed inputs. IEEE 1687-2014 §6.4.6.1 (printed p82,
PDF96) defines ScanInPort using `port_name`; §6.4.7 rules d/f (p111/PDF125)
requires width-matched scan-input connections, including slices.

`network.icl` selects one of two input bits, with inversion on the second
route. `one_hot.icl` routes one lane of a vector OneHotScanGroup into a
scalar scan chain through child vector ports. The corresponding PDL writes
and observes distinct values before and after selection changes.

The CLI regression checks ascending ranges, a nonzero single-bit input,
unknown/contending enables, and a 4096-bit input bus with a 64-bit register.
`guarded.icl` verifies that an implicit child's declared controls cannot be
ignored when scalar interface inference does not apply; a reset-only variant
remains supported.
The native scheduler test checks literal final state, input bit identity,
preservation of untouched unknown bits and rollback of failed reservations.
All successful CLI cases check source/flat-ICL/generated-PDL fixed points.
These cases have not been certified against Tessent. Multiple simultaneous
scan outputs and multiple TAP scheduling remain separate requirements.
