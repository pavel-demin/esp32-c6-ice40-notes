
`timescale 1 ns / 1 ps

module FDRE
(
  input  wire C,
  input  wire R,
  input  wire CE,
  input  wire D,
  output wire Q
);

  reg DATA = 1'b0;

  always @(posedge C)
  begin
    if(R) DATA <= 1'b0;
    else if(CE) DATA <= D;
  end

  assign Q = DATA;

endmodule
