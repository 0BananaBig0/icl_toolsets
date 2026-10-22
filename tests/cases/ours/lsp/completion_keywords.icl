// IEEE 1687-2014 6.3.3: keywords are also valid identifiers.
NameSpace Of;
Module KeywordTarget { DataInPort pin; }
NameSpace;
Module GlobalTarget {}
Module Completion {
  DataInPort Of;
  DataInPort Instance;
  DataInPort local_signal;
  DataOutPort result { Source Of ; }
  DataOutPort combined[1:0] { Source Instance, Of ; }
  LogicSignal enabled { Instance & Of ; }
  Instance child Of Of::KeywordTarget;
  Instance global Of ::GlobalTarget;
}
