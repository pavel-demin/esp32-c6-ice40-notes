module top
(
  input  wire clk_i,
  output wire led_o
);

  wire aclk;

  SB_GB buf_0 (
    .USER_SIGNAL_TO_GLOBAL_BUFFER(clk_i),
    .GLOBAL_BUFFER_OUTPUT(aclk)
  );

  reg [31:0] counter = 32'd0;

  always @(posedge aclk)
  begin
    counter <= counter + 1'b1;
  end

  assign led_o = counter[24];

endmodule
