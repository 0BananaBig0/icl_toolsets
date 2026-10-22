Module m {
  // iProc_namespace is written `(namespace_name? '::')? ref_module_name
  // ('::' sub_namespace)?` — the prefix is optional, so `::mod` names a
  // module in the global namespace.
  DataRegister dr {
    ReadCallBack ::mod p <D>;
    WriteCallBack ::mod::sub q <R> 5 "s" $X;
    WriteCallBack m::p <D>;
  }
  Instance i Of ::M { InputPort a = b; }
}
