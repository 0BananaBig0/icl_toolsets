Module Tap { CaptureEnPort se; ToCaptureEnPort toSe { Source se; } } Module Example { Instance tap_fsm Of Tap {} ToCaptureEnPort toSE; ToCaptureEnPort toSe2 { Source tap_fsm.toSe; } }
