.DEFAULT_GOAL := sim


verilog:
	make -C ./npc_soc sbt
	mkdir -p build || true
	iverilog -E -o ./build/ysyx_25080216.sv -s ysyx_25080216 ./generated_svsrc/*.sv

IMG ?= ./test_progs/rtthread-riscv32e-npc.bin


STA_DIR = $(NPC_HOME)/npc_soc/sta
SDC_FILE = $(STA_DIR)/npc.sdc
SCRIPT_FILE = $(STA_DIR)/sta.tcl

iverilog: verilog
	mkdir -p ./iv_temp/ || true
	objcopy -I binary -O verilog --verilog-data-width=4 $(IMG) ./iv_temp/temp.hex
	iverilog -o ./iv_temp/tb.vvp -s tb_npc ./build/ysyx_25080216.sv iverilog_tb/*.sv -g2012
	vvp -n ./iv_temp/tb.vvp

CLK_FREQ_MHZ = 1000
sta: verilog
	rm -rf ./sta
	mkdir -p ./sta
	make -C $(STA_HOME) sta \
	DESIGN=ysyx_25080216 SDC_FILE=$(SDC_FILE) PDK=nangate45 \
	CLK_FREQ_MHZ=$(CLK_FREQ_MHZ) CLK_PORT_NAME=clock O=$(NPC_HOME)/sta \
	RTL_FILES="$(NPC_HOME)/build/ysyx_25080216.sv"

PDK_CELLS ?= $(STA_HOME)/pdk/nangate45/sim/cells.v
NETLIST_PATH ?= $(shell find $(STA_DIR) -name "ysyx_25080216.netlist.v.sim")

iverilog-netlist: verilog
	mkdir -p ./iv_temp/ || true
	objcopy -I binary -O verilog --verilog-data-width=4 $(IMG) ./iv_temp/temp.hex
	iverilog -o ./iv_temp/tb.vvp -s tb_npc $(NETLIST_PATH) $(PDK_CELLS) iverilog_tb/*.sv -g2012
	vvp -n ./iv_temp/tb.vvp

clean:
	rm build/*.sv generated_svsrc/*.* iv_temp/* sta/*

.PHONY: clean verilog iverilog sta