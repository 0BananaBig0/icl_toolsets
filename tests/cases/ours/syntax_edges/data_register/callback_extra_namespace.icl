Module EdgeCallbackExtraNamespace {
  DataRegister value[7:0] {
    ReadCallBack lib::Device::operations::extra readValue <R>;
  }
}
