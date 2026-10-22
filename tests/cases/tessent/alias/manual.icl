// Tessent v2023.4 Alias example; overflowing 4'haa corrected to 4'ha.
Module instrument {
   DataInPort  in[8:0];
   DataOutPort out[7:0];
   ClockPort   clk;
  
   Alias write_en    = in[8]{ RefEnum YesNo; iApplyEndState 1'b0; }
   Alias enable      = in[7]           { RefEnum YesNo;    }
   Alias mode[3:0]   = in[6:5],in[3:2] { RefEnum Modes;    }
   Alias p1[2:0]     = in[4],in[1:0];
   Alias go          = out[0]          { RefEnum PassFail; }
   Alias done        = out[1]          { RefEnum YesNo;    }
   Alias count[5:0]  = out[7:2];
   Enum PassFail {
      Pass = 1'b1;
      Fail = 1'b0;  
   }
   Enum YesNo {
      Yes  = 1'b1;
      No   = 1'b0;
   }
   Enum Modes {
      none      = 4'b0 ;
      red       = 4'b0011 ;
      blue      = 4'ha ;
      green     = 4'd9 ;
   }
}
