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

  integer exu_delay_counter, idu_delay_counter;

  function integer next_clear(integer old_cnt, input ready, input valid);
    if (old_cnt != 'd0) return ready ? 'd0 : old_cnt + 'd1;
    else return {ready, valid} == 2'b01 ? old_cnt + 'd1 : 'd0;
  endfunction

  always @(posedge clock) begin
    if (reset) begin
      exu_delay_counter <= 'd0;
      idu_delay_counter <= 'd0;
    end else begin
      if (ifu_rready && ifu_rvalid) notify_ifu_event();
      if (lsu_rready && lsu_rvalid) notify_lsu_event();
      if (exu_delay_counter == 'd0 && exu_valid) notify_exu_event();
      if (idu_delay_counter == 'd0 && idu_valid) notify_idu_event();

      exu_delay_counter <= next_clear(exu_delay_counter, exu_ready, exu_valid);
      idu_delay_counter <= next_clear(idu_delay_counter, idu_ready, idu_valid);

    end
  end

endmodule
