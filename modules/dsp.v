
`timescale 1 ns / 1 ps

module dsp
(
  input  wire        CLK,

  input  wire [15:0] A,
  input  wire [15:0] B,

  output wire [31:0] P
);

  reg [31:0] int_p_reg;

  always @(posedge CLK)
  begin
    int_p_reg <= A * B;
  end

  assign P = int_p_reg;

endmodule
