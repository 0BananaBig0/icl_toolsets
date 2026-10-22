Module EdgeCallbackSplitMarker {
  DataRegister value[7:0] {
    WriteCallBack ::Device::operations writeValue < /* not one marker */ D>;
  }
}
