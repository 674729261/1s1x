CSRC=csrc/*.cpp
SVSRC=svsrc/*.sv
TOPNAME=RNG
BIN=./obj_dir/V$(TOPNAME)

CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""
VERILATORFLAGS += --cc --exe --build --trace -j 16

compile_chisel:
	sbt run

all: 
	verilator $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	-CFLAGS $(CXXFLAGS) -Wall $(CSRC) $(SVSRC) -Wno-UNUSEDSIGNAL

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN)

clean:
	rm -rf obj_dir
	sbt clean
	rm -f svsrc/*

.PHONY: all sim clean

include ../Makefile
