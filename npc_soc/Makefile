.DEFAULT_GOAL := sim

include $(NVBOARD_HOME)/scripts/nvboard.mk

NXDC = $(SOC_NPC_HOME)/nvboard/top.nxdc
NXDC_CPP_PATH = $(SOC_NPC_HOME)/nvboard/generated
NXDC_CPP = $(SOC_NPC_HOME)/nvboard/generated/auto_bind.cpp

CSRC = $(shell find csrc -name "*.cpp")
CSRC += /usr/local/lib/libreplxx.a $(NVBOARD_ARCHIVE)
CSRC += $(NXDC_CPP)

VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
VSRC += $(wildcard $(SOC_HOME)/build/*.v)
VSRC += $(shell find $(SOC_HOME)/perip -name *.v)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = ysyxSoCFull
BIN = $(MDIR)/V$(TOPNAME)

SV_FILELIST=generated_svsrc/filelist.f

SVSRC_FILES = $(shell cat $(SV_FILELIST))
SVSRC = $(patsubst %,$(SOC_NPC_HOME)/generated_svsrc/%,$(SVSRC_FILES))

VERILATOR = verilator
PYTHON = python

STA_DIR = $(SOC_NPC_HOME)/sta
SDC_FILE = $(STA_DIR)/npc.sdc
SCRIPT_FILE = $(STA_DIR)/sta.tcl

VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -Ofast   \
-march=native -mtune=native \
-funroll-loops -flto=auto -ffast-math -fno-math-errno -funsafe-math-optimizations 
VERILATOR_LDFLAGS = -flto=auto -fipa-pta 


V_CXXFLAGS += -CFLAGS "$(VERILATOR_CFLAGS) -std=c++23 -I$(SOC_NPC_HOME)/include -I$(NVBOARD_HOME)/usr/include" \
-LDFLAGS "-lSDL2 -lSDL2_image -lSDL2_ttf" 
# VERILATORFLAGS += --noassert --trace --no-coverage --x-assign fast --x-initial fast -O3 --threads 1 --cc --exe --build -j 8 
VERILATORFLAGS += --autoflush -I$(SOC_HOME)/perip/uart16550/rtl -I$(SOC_HOME)/perip/spi/rtl --trace \
--timescale "1ns/1ns" --no-timing -O3 --threads 1 --cc --exe --build -j 8 \
'-Dlayer$$Verifying' '-Dlayer$$PerformanceCounterLayer' '-Dlayer$$AXIAssertLayer'
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET -Wno-PINCONNECTEMPTY \
-Wno-ASSIGNDLY -Wno-DEFPARAM -Wno-UNUSEDPARAM -Wno-PROCASSINIT -Wno-UNOPTFLAT

FAST_FLAGS = -Ofast -march=native -mtune=native

-include scripts/*.mk

MDIR = $(SOC_NPC_HOME)/build/default/

npc: $(SV_FILELIST) $(LIBCAPSTONE) $(SO_PATH_NEMU)
	mkdir -p $(MDIR) || true
	$(VERILATOR)  $(VERILATORFLAGS) \
	--top-module $(TOPNAME) --public-flat-rw  \
	-CFLAGS -DTOP_NAME="\"V$(TOPNAME)\"" \
	--MAKEFLAGS "-j12 OPT_FAST=\"$(FAST_FLAGS)\"" \
	$(V_CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) $(VERILATOR_DISABLE_WARNINGS) \
	--Mdir $(MDIR)

sbt: $(SV_FILELIST)

$(SV_FILELIST): $(CHISELSRC)
	sbt run 
# 	bash -c "find generated_svsrc/ -name *.sv -exec sed -i 's+src/main/scala/empty/++g' {} \\;"



PROG ?= /home/shitful/1s1x/ysyx-workbench/am-kernels/tests/cpu-tests/build/fib-riscv32e-soc.bin
TIME_STAMP := $(shell date +"%Y_%m_%d_%H_%M_%S")
LOG_FILE ?= "log/$(TIME_STAMP).log"

$(NXDC_CPP): $(NXDC)
	mkdir -p $(NXDC_CPP_PATH) || true
	$(PYTHON) $(NVBOARD_HOME)/scripts/auto_pin_bind.py $(NXDC) $(NXDC_CPP)

sim: npc
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --log $(LOG_FILE) --difftest


nvboard: $(NXDC_CPP) nvboard-archive npc
	$(call git_commit, "sim RTL with nvboard")
	$(BIN) --image $(PROG) --log $(LOG_FILE) --difftest --nvboard

sta: sbt
	mkdir -p $(STA_DIR)/result || true
	make -C $(STA_HOME) sta \
	DESIGN=ysyx_25080216 SDC_FILE=$(SDC_FILE) PDK=nangate45 \
	CLK_FREQ_MHZ=4000 CLK_PORT_NAME=clock O=$(STA_DIR)/result \
	RTL_FILES="$(SVSRC)"

perf: LOG_FILE = /tmp/perf.log
perf: npc
# 	make -C $(AM_HOME)/../am-kernels/benchmarks/microbench ARCH=riscv32e-soc run mainargs=test LOG_FILE=$(LOG_FILE)
	cat /tmp/perf.log | grep -oP "Total simulated clock periods : \d+"
	cat /tmp/perf.log | grep -oP "Total simulated instructions : \d+"
	cat /tmp/perf.log | grep -oP "Clocks per instruction : \d+(\.\d+)?"
	cat $(shell find $(STA_DIR)/result -name "ysyx_25080216.rpt") | grep "Freq" -A 2 | grep -oP "\|\s*\d+(\.\d+)\s*\|\s*$" | grep -oP "\d+(\.\d+)?"
	


valgrind: FAST_FLAGS = -O0 -g
valgrind: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O0 -g
valgrind: MDIR = ./build/valgrind/
valgrind: npc
	valgrind --leak-check=full --show-leak-kinds=all \
	 $(BIN) --image $(PROG) --log $(LOG_FILE) --difftest --batch 



gdb: FAST_FLAGS = -O0 -g
gdb: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O0 -g
gdb: MDIR = ./build/gdb/
gdb: npc
	gdb --args $(BIN) --image $(PROG) --log $(LOG_FILE) --difftest



clean:
	rm -rf build
	sbt clean
	rm -rf generated_svsrc/*

clean_log:
	rm -rf $(SOC_NPC_HOME)/log/*

clean_sta:
	rm -rf $(STA_DIR)/result/*



.PHONY: npc sim nvboard sta clean clean_log clean_sta sbt gdb valgrind perf

include ../Makefile
