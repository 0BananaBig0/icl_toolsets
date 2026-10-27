// Self-constructed experiment for IEEE 1687-2014 6.4.15 source widths.
Module ByteSource {
  DataInPort data[7:0];
  DataInPort enable;
  DataOutPort value[7:0] { Source data; Enable enable; }
}
Module WidthProbe {
  DataInPort data[7:0];
  DataInPort enable;
  Instance driver Of ByteSource {
    InputPort data = data;
    InputPort enable = enable;
  }
  OneHotDataGroup wide[15:0] { Port driver.value; }
  DataOutPort observed[15:0] { Source wide; }
}
Module EqualWidthProbe {
  DataInPort data[7:0];
  DataInPort enable;
  Instance driver Of ByteSource {
    InputPort data = data;
    InputPort enable = enable;
  }
  OneHotDataGroup wide[7:0] { Port driver.value; }
  DataOutPort observed[7:0] { Source wide; }
}
Module AscendingWidthProbe {
  DataInPort data[7:0];
  DataInPort enable;
  Instance driver Of ByteSource {
    InputPort data = data;
    InputPort enable = enable;
  }
  OneHotDataGroup wide[0:15] { Port driver.value; }
  DataOutPort observed[0:15] { Source wide; }
}
