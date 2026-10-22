Module M { DataInPort b[1:0]; Alias a = b { iApplyEndState 1'b0, ~1'b0; } Alias c = b { iApplyEndState 1 + 1; } }
