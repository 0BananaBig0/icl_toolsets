# Raw scan reservations

Self-constructed IEEE1687-2014 §7.9.19 single-stream reservation example, MIT.
A take followed by an applied write establishes bit3 protection. Explicit raw SI
updates that protected bit; the later partial write must preserve its new value.
The explicit raw SO=8 and final value=A7 comparisons remain. Unwritten takes do
not acquire protection from raw scans. No vendor extension or harness exception.

All ordinary writes explicitly compare the pre-write value (reset0, thenA5).
These author observations retain the comparisons Tessent emits in forward replay.
An initial source qualification showed Tessent2026.2 keeping the previous iWrite
reservation after the raw update and rejecting the next partial write. This is
retained as a reverse qualification limitation; no reverse certificate is claimed.
