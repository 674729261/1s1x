CSRC = $(wildcard csrc/*.cpp)
CSRC += /usr/local/lib/libreplxx.a
SVSRC = generated_svsrc/CPU.sv
VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

VERILATOR = verilator

CXXFLAGS += -CFLAGS "-DSPDLOG_USE_STD_FORMAT -O3 -I../include -std=c++23" -CFLAGS -DTOP_NAME="\"V$(TOPNAME)\""
VERILATORFLAGS += --cc --exe --build --trace -j 16 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET


all: $(SVSRC)
	$(VERILATOR) $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	$(CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) $(VERILATOR_DISABLE_WARNINGS) 

$(SVSRC): $(CHISELSRC)
	sbt run

PROG ?= test_progs/add-minirv-npc.bin
MEM_SIZE ?= 134217728
TIME_STAMP = $(shell date +"%Y_%m_%d_%H_%M_%S")
LOG_FILE ?= "log/$(TIME_STAMP).log"
sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE)

clean:
	rm -rf obj_dir
	sbt clean
	rm -f generated_svsrc/*

clean_log:
	rm -rf log/*

.PHONY: all sim clean

include ../Makefile
