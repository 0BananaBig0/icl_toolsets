# Merge fixtures

Self-constructed, complete ICL/PDL examples exercising IEEE 1687-2014 §7.9.18:
independent iCalls may share an apply, and direct iNotes execute before calls.
The Python tests add nested, conflicting, barrier, invalid and local-reset
variants. These are not Tessent-qualified goldens.
