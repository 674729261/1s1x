import "DPI-C" function void notify_ifu_event();
import "DPI-C" function void notify_lsu_event();
import "DPI-C" function void notify_exu_event();
import "DPI-C" function void notify_idu_event();


module PerformanceCounter (
    input clock,
    input reset,
    input ifu_rready,
    input ifu_rvalid,
    input lsu_rready,
    input lsu_rvalid,
    input exu_ready,
    input exu_valid,
    input idu_ready,
    input idu_valid
);

  reg exu_clear, idu_clear;

  function reg next_clear(input old_clear, input ready, input valid);
    if (old_clear) return ready ? 1'b0 : 1'b1;
    else return {ready, valid} == 2'b01;
  endfunction

  always @(posedge clock) begin
    if (reset) begin
      exu_clear <= 1'b0;
      idu_clear <= 1'b0;
    end else begin
      if (ifu_rready && ifu_rvalid) notify_ifu_event();
      if (lsu_rready && lsu_rvalid) notify_lsu_event();
      if (exu_clear && exu_valid) notify_exu_event();
      if (idu_clear && idu_valid) notify_idu_event();

      exu_clear <= next_clear(exu_clear, exu_ready, exu_valid);
      idu_clear <= next_clear(idu_clear, idu_ready, idu_valid);

    end
  end

endmodule
