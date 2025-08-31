CSRC = $(wildcard csrc/*.cpp)
SVSRC = generated_svsrc/CPU.sv
VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

VERILATOR = verilator

CXXFLAGS += -CFLAGS -std=c++23 -CFLAGS -DTOP_NAME="\"V$(TOPNAME)\"" -CFLAGS -I./include
VERILATORFLAGS += --cc --exe --build --trace -j 16 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET


all: $(SVSRC) $(STD_MODULE)
	$(VERILATOR) $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	$(CXXFLAGS) -Wall $(STD_MODULE) $(CSRC) $(SVSRC) $(VSRC) \
	$(VERILATOR_DISABLE_WARNINGS)

$(SVSRC): $(CHISELSRC)
	sbt run

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) $(PROG)

clean:
	rm -rf obj_dir
	sbt clean
	rm -f generated_svsrc/*

.PHONY: all sim clean

include ../Makefile
