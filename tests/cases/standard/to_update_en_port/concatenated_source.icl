// IEEE §6.4.6.10 concat source grammar; Honza revision 333f8bc narrows this to one signal.
Module Example { UpdateEnPort input[1:0]; ToUpdateEnPort output[1:0] { Source input[1], input[0]; } }
