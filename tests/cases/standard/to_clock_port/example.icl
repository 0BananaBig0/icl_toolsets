// IEEE 1687-2014 §6.4.6.20 printed 103–104/PDF 117–118; completed example.
Module Example { ClockPort SCK; ToClockPort FastCLK { Source SCK; FreqMultiplier 3; FreqDivider 2; } ToClockPort SlowCLK { Source SCK; FreqDivider 2; } ToClockPort CLK { Period 3400ps; } }
