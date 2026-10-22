// IEEE 1687-2014 §6.4.6.16 printed 99–100/PDF 113–114; completed example.
Module bottom_die { TMSPort tms; DataInPort enable; DataMux escalator_active SelectedBy enable { 1'b0 : tms; 1'b1 : 1'b0; } ToTMSPort tms_to_upper_die { Source escalator_active; } }
