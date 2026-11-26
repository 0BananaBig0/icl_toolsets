Module DataStateSnapshot {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] { WriteDataSource data; WriteEnSource enable; ResetValue 0; }
 DataOutPort q[3:0] { Source value; }
}
