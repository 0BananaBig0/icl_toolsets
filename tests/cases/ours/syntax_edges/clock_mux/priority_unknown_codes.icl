Module EdgeClockMuxPriorityUnknownCodes {
  DataInPort select[1:0];
  ClockPort primary;
  ClockPort backup;
  ClockMux route SelectedBy select {
    1'b0, 1'b1 | 2'h2 : primary;
    2'BXX : backup;
  }
  ToClockPort out { Source route; }
}
