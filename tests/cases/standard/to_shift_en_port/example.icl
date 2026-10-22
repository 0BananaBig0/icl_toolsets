Module Tap { ShiftEnPort se; ToShiftEnPort toSe { Source se; } } Module Example { Instance tap_fsm Of Tap {} ToShiftEnPort toSE; ToShiftEnPort toSe2 { Source tap_fsm.toSe; } }
