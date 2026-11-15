# Capture/update mux selector operation

Self-constructed IEEE1687-2014 6.4.6.4(b),6.4.6.5(b),6.4.6.9/.10 fixtures. CaptureSource1010 differs from reset0000. The update case uses a separately gated UpdateEn input. Both use nested standalone clients and explicit parent wiring.

Tessent2026.2 forward certification passes for both cases using the unchanged resolved-bits ordered-apply comparator. The certificates retain staged inputs, runner hashes, raw exports and local output under tests/golden/tessent_roundtrip/{capture_mux_selector,update_mux_selector}-forward. Native tests also cover routed data observations, ownership, expected-value comparisons and a scan-driven prerequisite.

Reverse attempts are preserved as failures: capture exports but its round-trip groups differ; update source export rejects automatic suppression-state assignment under the default vendor policy and points to allow_capture_update_assignment. Neither reverse direction is certified. These results qualify the recorded forward models and inputs, not general independent TAP phases or all capture/update gating.

Remote evidence: lianghuaxiao /tmp/icl-control-mux-operations-20261115; local fetched bundles: build/control-mux-operation-vendor/evidence. All19 actual staged file hashes match the submitted archive. No comparator policy or solver bound was changed.
