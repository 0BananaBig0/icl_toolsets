Module EdgeClockMuxConcatXorAlternative {
  DataInPort select[1:0];
  ClockPort primary;
  ClockMux route SelectedBy select { 1'b0, 1'b1 ^ 2'h2 : primary; }
}
