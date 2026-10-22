Module EdgeCallbackAdjacentArguments {
  DataRegister value[7:0] {
    WriteCallBack ::Device::operations writeValue <D><R>8'hFF"tail";
  }
}
