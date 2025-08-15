CSRC=csrc/main.cpp
VSRC=vsrc/example.v
BIN=./obj_dir/Vexample

all:
	verilator --cc --exe --build --trace -j 16 -Wall $(CSRC) $(VSRC)

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN)

clean:
	rm -rf obj_dir

.PHONY: all sim clean

include ../Makefile
