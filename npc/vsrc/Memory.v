module Memory (
    valid,
    wen,
    waddr,
    raddr,
    wmask,
    wdata,
    rdata
);
  import "DPI-C" function int pmem_read(input int raddr);
  import "DPI-C" function void pmem_write(
    input int  waddr,
    input int  wdata,
    input byte wmask
  );
  input valid, wen;
  input [31:0] waddr, raddr, wdata;
  input [3:0] wmask;
  output reg [31:0] rdata;
  always @(*) begin
    if (valid) begin  // 有读写请求时
      rdata = pmem_read(raddr);
      if (wen) begin  // 有写请求时
        pmem_write(waddr, wdata, {4'h0, wmask});
      end
    end else begin
      rdata = 0;
    end
  end
endmodule
