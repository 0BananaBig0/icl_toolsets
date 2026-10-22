// Tessent v2023.4 Command_Toclockport_ide389c67c.html, clockGen example reduced to its clock interface.
Module clockGen { ClockPort refClkA; Parameter sclkMult = 1; Parameter sclkDiv = 1; ToClockPort sclk { FreqMultiplier $sclkMult; FreqDivider $sclkDiv; Source refClkA; } }
