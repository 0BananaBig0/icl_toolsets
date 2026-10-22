// IEEE §6.4.6.9 concat source grammar; Honza revision 333f8bc narrows this to one signal.
Module Example { CaptureEnPort input[1:0]; ToCaptureEnPort output[1:0] { Source input[1], input[0]; } }
