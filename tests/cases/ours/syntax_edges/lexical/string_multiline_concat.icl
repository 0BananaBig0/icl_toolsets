Module EdgeStringMultilineConcat {
  Parameter HEAD = "line one
line two";
  Parameter TEXT = $HEAD, /* separator */ "\q\;\{\}", "";
  Attribute message = $TEXT;
}
