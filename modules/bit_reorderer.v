
`timescale 1 ns / 1 ps

module bit_reorderer #
(
  parameter integer DATA_WIDTH = 32
)
(
  input  wire [DATA_WIDTH-1:0] in_data,

  output wire [DATA_WIDTH-1:0] out_data
);

  genvar j, k;

  generate
    for(j = 0; j < DATA_WIDTH / 8; j = j + 1)
    begin : BYTES
        for(k = 0; k < 8; k = k + 1)
        begin : BITS
          assign out_data[j*8+k] = in_data[j*8+7-k];
        end
    end
  endgenerate

endmodule
