// IEEE 1687-2014 §6.4.6.14 printed 97/PDF 111; completed example.
Module Example { ResetPort myreset_signal { ActivePolarity 0; } ToResetPort local_reset_out { Source myreset_signal; ActivePolarity 0; } }
