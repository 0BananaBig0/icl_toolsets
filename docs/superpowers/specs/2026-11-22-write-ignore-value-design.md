# iWrite option payload correctness

Task146, parent master c558013. An independently reproduced existing defect
turns a requested primary write1 into emitted0 with a legal trailing
-ignore_when_value_already_present auto/off/on option. The editor validates
the syntax, Runtime retains four operands, and downstream literal-access
consumers expect target/value pairs; the supplied value becomes an X mask.

Normalize iWrite to its validated target and supplied value at the existing
Runtime operation boundary. Reuse the shared frontend validation before dropping
the trailing optimization hint. Do not reevaluate Tcl arguments, alter original
SourceAction arguments/procedure text, change read-without-compare semantics,
or duplicate normalization in each downstream consumer. This reaches history,
merge, reset planning, reservations, normal/physical generation and replay.

The local Tessent Shell Reference Manual v2023.4 iWrite topic
Command_Iwrite_id82cc6234.html specifies a required second value argument;
the optional switch permits omitting an already-satisfied register load for on,
requires a load for off and uses retargeting policy for auto. The native
generator retains its existing explicit-load policy for all modes. Permission
to omit a load is not a promise to eliminate one. No new skip-load optimizer,
global option command, clock/event behavior, dependency or metadata is added.
Broader scheduling optimization remains item10. Vendor command context is
patterns -ijtag / analysis with an open pattern set (other contexts in topic);
native CLI examples use the repository runtime, not a live device.

Acceptance: all allowed modes preserve numeric, partial, alias and enum payloads;
scan/parallel DataRegister goals and merged calls retain requested values;
invalid optioned values reject before publication even if reset discards queue;
Tcl arguments evaluate once; source/flat/generated replay agrees. Native Runtime
asserts normalized operation pairs and original trace arguments, plus numeric
validation in a queue with reads. New maintained tests stay under tests/.

Performance: constant-time removal of the two optional operands after existing
validation, with no full-design scan, subprocess or additional solver work.
Measure representative16/128 applies of32-bit pin writes, three repeats of
source/flat/generated forms with matched parent/current unflagged controls.
Retain limits and report actual wall time/RSS without a speedup claim.

Run focused checks then required native/dynamic/static/SAN/quality/package gates,
freeze identities, finish the ledger and request one fresh independent branch
review. Integrate/push/delete the completed branch; continue remaining items.
