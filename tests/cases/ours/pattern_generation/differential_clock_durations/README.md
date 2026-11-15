# Differential clock duration probe

Self-constructed from IEEE1687-2014 6.4.6.19/.20,7.9.15/.17: a root
input pair, a3/2 child output pair and an embedded10ns oscillator pair.
No RTL or externally sourced example is included. This tests relative cycle
counts on clock-only paths, not phase/waveform or hardware-event simulation.

Stage model.icl and one PDL as input.pdl beside
[the existing-style Tessent probe](../../../../src/pattern/probe_differential_clock_durations.tcl).
Run source.pdl and locally normalized output in separate fresh directories.
The script declares both physical clock inputs at100ns and root_n as the
inverse input of root, exports
result.pdl and preserves errors.
Use actual fetched input/runner hashes and compare ordered clock/wait traces;
a direct probe alone is not a directional golden certificate.
