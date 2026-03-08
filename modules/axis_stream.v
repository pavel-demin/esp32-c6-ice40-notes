
`timescale 1 ns / 1 ps

module axis_stream #
(
  parameter integer AXIS_TDATA_WIDTH = 32
)
(
  input  wire                        aclk,
  input  wire                        aresetn,

  input  wire [15:0]                 cfg_data,

  output wire [AXIS_TDATA_WIDTH-1:0] m_axis_tdata,
  output wire                        m_axis_tvalid,
  input  wire                        m_axis_tready
);

  reg [15:0] int_cntr_reg = 16'd0;
  reg [AXIS_TDATA_WIDTH-1:0] int_data_reg = {(AXIS_TDATA_WIDTH){1'b0}};

  wire int_valid_wire;

  assign int_valid_wire = int_cntr_reg == cfg_data;

  always @(posedge aclk)
  begin
    if(~aresetn)
    begin
      int_cntr_reg <= 16'd0;
      int_data_reg <= {(AXIS_TDATA_WIDTH){1'b0}};
    end
    else
    begin
      if(int_valid_wire)
      begin
        int_cntr_reg <= 16'd0;
        int_data_reg <= int_data_reg + 1'b1;
      end
      else
      begin
        int_cntr_reg <= int_cntr_reg + 1'b1;
      end
    end
  end

  assign m_axis_tdata = int_data_reg;
  assign m_axis_tvalid = int_valid_wire;

endmodule
