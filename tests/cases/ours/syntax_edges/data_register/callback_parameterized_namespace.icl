Module EdgeCallbackParameterizedNamespace {
  Parameter TARGET = "Device";
  Parameter SUB = "operations";
  Parameter PROC = "readValue";
  Parameter ADDRESS = 4;
  DataRegister value[7:0] {
    ReadCallBack lib /* namespace */ :: $TARGET :: $SUB $PROC
      <R> ($ADDRESS + (2 * 3)) "quoted \"argument\"";
  }
}
