# Raw IR through one bound TAP

Self-constructed, MIT, adapted from the local AccessLink probe. BSDL IR width4,
IJTAG_ON0011/IJTAG_OFF0100 and capture0001 are explicit fixture properties.
IEEE1687-2014 §7.9.12 and IEEE1149.1-2001(R2008) IR behavior apply. Known
binary raw IR is followed by ordinary data access and an instruction change.
No unknown-SI, multi-TAP, raw DR, parallel or new clock/reset semantics claimed.
