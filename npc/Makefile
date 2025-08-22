CSRC = $(wildcard csrc/*.cpp)
SVSRC = $(wildcard generated_svsrc/*.sv)
VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

CXXFLAGS += -DTOP_NAME="\"V$(TOPNAME)\""
VERILATORFLAGS += --cc --exe --build --trace -j 16 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET

$(SVSRC):
	sbt run

all: $(SVSRC)
	verilator $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	-CFLAGS $(CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) $(VERILATOR_DISABLE_WARNINGS)

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) $(PROG) 65536

clean:
	rm -rf obj_dir
	sbt clean
	rm -f svsrc/*

.PHONY: all sim clean

include ../Makefile
