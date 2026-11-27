# Preserve pending observations across ordinary resets (Task199)

This repairs the existing completion-program observation contract (original
items 3/6): preparation must preserve unread storage, and actual comparisons
must remain independent of expected data. The protected Task197 and Task198
binaries both accept a scan preparation that enables a primary read while an
acyclic DataRegister reset clears its source; actual readback becomes 0 instead
of the original 1. This is a correctness bug, not new topology support.

Compare each requested logical bit at its selected capture against a projection
of the original storage under the actual capture controls. Carry that original
snapshot across implicit TAP instruction setup. Protect direct ScanRegister
CaptureSource bits as well as logical data reads. Selectors below a logical
target, including OneHot selection, contribute observed data; unrelated bits
and inactive physical observation-route arms do not. Forwarding-port availability
uses current capture controls, while OneHot payloads retain original selection
and enables. Actual values remain independent of expected comparison bits.

Reuse the symbolic network cache with one optional borrowed access context;
existing callers retain the default behavior. Each read frame copies its
assigned-node map and evaluates only requested dependencies; a OneHot payload
view is created lazily when needed. Keep existing intermediate feedback checks,
capture-before-update semantics and resource bounds. No new protocol, dependency
or event format is introduced; this adds an internal optional C++ constructor
argument without changing CLI, PDL or observation interfaces.

Reject impossible preparations atomically; preserve state, observations and
existing output files. Safe preparation, capture before a later destructive
update, unknown comparisons and actual mismatches must continue to work.

Implementation proceeds inline under the existing completion-program approval,
with one fresh whole-branch reviewer and one test-first Critical/Important
repair pass. Broad inventory, item 1 exclusion, 117 stashes and 44 inherited
Minors remain unchanged. New live-device or vendor qualification is unclaimed.
