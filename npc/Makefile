.DEFAULT_GOAL := sim

verilog:
	make -C ./npc_soc sbt

IMG ?= ./test_progs/microbench-riscv32e-npc.bin

iverilog: verilog
	mkdir -p ./iv_temp/|| true
	objcopy -I binary -O verilog --verilog-data-width=4 $(IMG) ./iv_temp/temp.hex
	iverilog -E -o ./iv_temp/pp.sv -s tb_npc ./build/*.sv iverilog_tb/*.sv
	iverilog -o ./iv_temp/tb.vvp -s tb_npc ./iv_temp/pp.sv -g2012
	vvp -n ./iv_temp/tb.vvp

clean:
	rm build/*.sv

.PHONY: clean verilog iverilog