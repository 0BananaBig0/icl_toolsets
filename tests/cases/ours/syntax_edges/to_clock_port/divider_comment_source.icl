Module EdgeClockDividerCommentSource {
  ClockPort input;
  ToClockPort out {
    Source /* clock */ input;
    FreqDivider /* divide */ 2 /* factor */;
  }
}
