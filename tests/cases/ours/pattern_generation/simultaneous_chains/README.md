# Simultaneous client chains

Self-constructed IEEE 1687-2014 §6.4.16 fixture. Two named chains share
capture/shift/update controls; one holds four bits and the other two bits.
The source explicitly reads initial reset values so vendor-added comparisons
are not mistaken for ordered-apply equivalence.

Only certificates produced by the unchanged round-trip comparator qualify a
direction; preparation and a successful vendor export alone do not.
