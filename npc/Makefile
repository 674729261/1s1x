CSRC = $(wildcard csrc/*.cpp)
SVSRC = generated_svsrc/CPU.sv
VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

VERILATOR = verilator

CXXFLAGS += -CFLAGS "-O3 -I../include -std=c++23" -CFLAGS -DTOP_NAME="\"V$(TOPNAME)\""
VERILATORFLAGS += --cc --exe --build --trace -j 16 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET


all: $(SVSRC)
	$(VERILATOR) $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	$(CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) \
	$(VERILATOR_DISABLE_WARNINGS)

$(SVSRC): $(CHISELSRC)
	sbt run

PROG ?= test_progs/add-minirv-npc.bin
CYCLES ?= -1
MEM_SIZE ?= 134217728
sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --cycles $(CYCLES) --mem_size $(MEM_SIZE)

clean:
	rm -rf obj_dir
	sbt clean
	rm -f generated_svsrc/*

.PHONY: all sim clean

include ../Makefile
