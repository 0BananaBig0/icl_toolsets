Module GPIO_IP {
    Attribute IEEE1687_INSTRUMENT = "YES";
    ScanInPort TDI { Attribute ALT_NAME="GPIO_TDI"; }
    ScanOutPort TDO {
    Source tdr_GPIO[9];
    }
    ShiftEnPort SHIFT_EN;
    CaptureEnPort CAPTURE;
    UpdateEnPort UPDATE;
    SelectPort IP_SELECT;
    ResetPort TRST;
    TCKPort TCLK { Attribute ALT_NAME="GPIO_TCLK"; }
    ClockPort CLOCK;
    DataInPort RESET { Attribute REQUIRE_LO="YES"; }
    DataOutPort IP_ACCESSED { Attribute ALT_NAME="GPIO_IP_ACCESSED"; }
    ScanRegister tdr_GPIO[9:0] {
    ScanInSource TDI;
    }
    ScanInterface GPIO_scan {
    Port TDI;
    Port TDO;
    Port IP_SELECT;
    }
}
