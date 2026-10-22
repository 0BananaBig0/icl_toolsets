NameSpace ;
UseNameSpace ;
Module pll {
    ClockPort REFCLK;
    ToClockPort CLK1 {
        Source REFCLK;
        FreqMultiplier 2;
    }
    ToClockPort CLK2 {
        Source REFCLK;
        FreqMultiplier 10;
    }
    ToClockPort CLK3 {
        Source REFCLK;
        FreqMultiplier 2;
    }
    ToClockPort pll_out1 {
        Source REFCLK;
        FreqMultiplier 2;
    }

}
