# Standalone client controls

Self-constructed fixture for IEEE 1687-2014 §6.4.16, §6.7 and §6.8.
The case writes and reads a four-bit register through an explicit client
interface.

`cases.json` stages fresh forward/reverse Tessent qualification runs. A prepared
case or successful export alone is not a qualification certificate; record each
direction only after the unchanged ordered-apply comparator succeeds.

An initial parallel-storage probe connected UpdateEnPort directly to a
DataRegister WriteEnSource. Tessent 2026.2 rejected that connection (ICL65-1),
so it is excluded from this manifest and has no qualification certificate.
The synthetic state-model regression in `test_client_scan_controls.py` retains
that connection to detect symbolic/concrete phase disagreement; it does not
establish that the connection is valid IEEE ICL.
