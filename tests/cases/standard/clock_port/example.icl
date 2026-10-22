// IEEE 1687-2014 §6.4.6.19 printed 102–103/PDF 116–117; completed example.
Module Example { ClockPort PCK; ClockPort NCK { DifferentialInvOf PCK; } }
