module top
(
  input  wire clk_i,
  input  wire spi_sclk_i,
  input  wire spi_ssel_i,
  input  wire spi_mosi_i,
  output wire spi_miso_o,
  output wire led_o
);

  wire aclk, spi_sclk;

  wire [31:0] spi_s_axis_tdata;
  wire spi_s_axis_tvalid, spi_s_axis_tready;

  wire [31:0] spi_m_axis_tdata;
  wire spi_m_axis_tvalid, spi_m_axis_tready;

  wire [63:0] cfg_data;
  wire [31:0] sts_data;

  wire [31:0] s00_axis_tdata;
  wire s00_axis_tvalid, s00_axis_tready;

  SB_GB buf_0 (
    .USER_SIGNAL_TO_GLOBAL_BUFFER(clk_i),
    .GLOBAL_BUFFER_OUTPUT(aclk)
  );

  SB_GB buf_1 (
    .USER_SIGNAL_TO_GLOBAL_BUFFER(spi_sclk_i),
    .GLOBAL_BUFFER_OUTPUT(spi_sclk)
  );

  axis_spi spi_0 (
    .spi_sclk(spi_sclk),
    .spi_ssel(spi_ssel_i),
    .spi_mosi(spi_mosi_i),
    .spi_miso(spi_miso_o),
    .aclk(aclk),
    .aresetn(1'b1),
    .s_axis_tdata(spi_s_axis_tdata),
    .s_axis_tvalid(spi_s_axis_tvalid),
    .s_axis_tready(spi_s_axis_tready),
    .m_axis_tdata(spi_m_axis_tdata),
    .m_axis_tvalid(spi_m_axis_tvalid),
    .m_axis_tready(spi_m_axis_tready)
  );

  axis_hub #(
    .CFG_DATA_WIDTH(64),
    .STS_DATA_WIDTH(32)
  ) hub_0 (
    .aclk(aclk),
    .aresetn(1'b1),
    .s_axis_tdata(spi_m_axis_tdata),
    .s_axis_tvalid(spi_m_axis_tvalid),
    .s_axis_tready(spi_m_axis_tready),
    .m_axis_tdata(spi_s_axis_tdata),
    .m_axis_tvalid(spi_s_axis_tvalid),
    .m_axis_tready(spi_s_axis_tready),
    .cfg_data(cfg_data),
    .sts_data(sts_data),
    .s00_axis_tdata(s00_axis_tdata),
    .s00_axis_tvalid(s00_axis_tvalid),
    .s00_axis_tready(s00_axis_tready)
  );

  dsp dsp_0 (
    .CLK(aclk),
    .A(cfg_data[47:32]),
    .B(cfg_data[63:48]),
    .P(sts_data)
  );

  axis_counter #(
    .AXIS_TDATA_WIDTH(32)
  ) cntr_0 (
    .aclk(aclk),
    .m_axis_tdata(s00_axis_tdata),
    .m_axis_tvalid(s00_axis_tvalid),
    .m_axis_tready(s00_axis_tready)
  );

  assign led_o = cfg_data[0];

endmodule
