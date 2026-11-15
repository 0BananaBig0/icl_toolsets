# Inverted clock duration probe

Self-constructed from IEEE1687-2014 6.3.12,6.4.6.20 and7.9.15–17.
Explicit inverse aliases/output edges and nested ratios exercise relative
frequency counts; an override keeps old checked durations until a new iClock.
No phase/event or RTL simulation is claimed.

Reuse tests/src/pattern/probe_differential_clock_durations.tcl from the repository
root, staging model.icl and one PDL as input.pdl. The otherwise unused root_n
provides the existing runner's declared physical differential input pair.
Run original and normalized PDL in separate directories; retain actual hashes,
errors and ordered waits. This fixture is not itself a vendor certificate.
