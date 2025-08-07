PROJ_PATH = $(shell pwd)
SHELL := /bin/bash

O ?= $(PROJ_PATH)/result
SDC_FILE ?= $(PROJ_PATH)/scripts/default.sdc
export CLK_FREQ_MHZ ?= 500
export CLK_PORT_NAME ?= clk
PDK = nangate45


SCRIPT_DIR = $(PROJ_PATH)/scripts
# NETLIST_SYN_V   = $(RESULT_DIR)/$(DESIGN).netlist.syn.v
# NETLIST_FIXED_V = $(RESULT_DIR)/$(DESIGN).netlist.fixed.v
# TIMING_RPT = $(RESULT_DIR)/$(DESIGN).rpt
$(SRC_AUTO_BIND): $(NXDC_FILES)
	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

init:
	bash -c "$$(wget -O - https://ysyx.oscc.cc/slides/resources/scripts/init-yosys-sta.sh)"


e2: DESIGN=top_e2
e2: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e2: RTL_FILES="example/e2/EXP2.v"
e2: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e2: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e2: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt

e2: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e2/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -
# 	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V) $(DESIGN) $(PDK) $(NETLIST_FIXED_V) 2>&1 | tee $(RESULT_DIR)/fix-fanout.log
# 	echo tcl $(SCRIPT_DIR)/yosys-area.tcl $(DESIGN) $(PDK) $(NETLIST_FIXED_V) | yosys -l $(@D)/yosys-fixed.log -s -
# 	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V) $(DESIGN) $(PDK) 2>&1 | tee $(RESULT_DIR)/sta.log


e3: DESIGN=ALU
e3: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e3: RTL_FILES="example/e3/EXP3.v"
e3: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e3: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e3: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt

e3: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e3/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -
# 	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V) $(DESIGN) $(PDK) $(NETLIST_FIXED_V) 2>&1 | tee $(RESULT_DIR)/fix-fanout.log
# 	echo tcl $(SCRIPT_DIR)/yosys-area.tcl $(DESIGN) $(PDK) $(NETLIST_FIXED_V) | yosys -l $(@D)/yosys-fixed.log -s -
# 	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V) $(DESIGN) $(PDK) 2>&1 | tee $(RESULT_DIR)/sta.log

e4: DESIGN=top_e4
e4: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e4: RTL_FILES="example/e4/EXP4.v"
e4: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e4: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e4: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt
e4: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e4/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V) $(DESIGN) $(PDK) $(NETLIST_FIXED_V) 2>&1 | tee $(RESULT_DIR)/fix-fanout.log
	echo tcl $(SCRIPT_DIR)/yosys-area.tcl $(DESIGN) $(PDK) $(NETLIST_FIXED_V) | yosys -l $(@D)/yosys-fixed.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V) $(DESIGN) $(PDK) 2>&1 | tee $(RESULT_DIR)/sta.log

e6: DESIGN=top_e6
e6: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e6: RTL_FILES="example/e6/rng.v"
e6: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e6: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e6: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt
e6: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e6/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V) $(DESIGN) $(PDK) $(NETLIST_FIXED_V) 2>&1 | tee $(RESULT_DIR)/fix-fanout.log
	echo tcl $(SCRIPT_DIR)/yosys-area.tcl $(DESIGN) $(PDK) $(NETLIST_FIXED_V) | yosys -l $(@D)/yosys-fixed.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V) $(DESIGN) $(PDK) 2>&1 | tee $(RESULT_DIR)/sta.log


e7: DESIGN=top_e7
e7: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e7: RTL_FILES="example/e7/top_e7.v"
e7: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e7: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e7: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt
e7: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e7/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/fix-fanout.tcl $(SDC_FILE) $(NETLIST_SYN_V) $(DESIGN) $(PDK) $(NETLIST_FIXED_V) 2>&1 | tee $(RESULT_DIR)/fix-fanout.log
	echo tcl $(SCRIPT_DIR)/yosys-area.tcl $(DESIGN) $(PDK) $(NETLIST_FIXED_V) | yosys -l $(@D)/yosys-fixed.log -s -
	set -o pipefail && ./bin/iEDA -script $(SCRIPT_DIR)/sta.tcl $(SDC_FILE) $(NETLIST_FIXED_V) $(DESIGN) $(PDK) 2>&1 | tee $(RESULT_DIR)/sta.log


e8: DESIGN=top_e8
e8: RESULT_DIR=$(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
e8: RTL_FILES="example/e8/top_e8.v"
e8: NETLIST_SYN_V=$(RESULT_DIR)/$(DESIGN).netlist.syn.v
e8: NETLIST_FIXED_V=$(RESULT_DIR)/$(DESIGN).netlist.fixed.v
e8: TIMING_RPT=$(RESULT_DIR)/$(DESIGN).rpt
e8: $(RTL_FILES) $(SCRIPT_DIR)/yosys.tcl $(SCRIPT_DIR)/sta.tcl $(SDC_FILE)
	make -C example/e8/ run
	mkdir -p $(O)/$(DESIGN)-$(CLK_FREQ_MHZ)MHz
	echo tcl $(SCRIPT_DIR)/yosys.tcl $(DESIGN) $(PDK) \"$(RTL_FILES)\" $(NETLIST_SYN_V) | yosys -l $(@D)/yosys.log -s -


include $(NVBOARD_HOME)/scripts/nvboard.mk

clean:
	-rm -rf e2 result/
	make -C example/e2/ clean
	make -C example/e3/ clean
	make -C example/e4/ clean
	make -C example/e6/ clean
	make -C example/e7/ clean

