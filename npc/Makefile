CSRC = $(shell find csrc -name "*.cpp")
CSRC += /usr/local/lib/libreplxx.a

VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

SV_FILELIST=generated_svsrc/filelist.f

SVSRC =  $(shell cat $(SV_FILELIST) | sed 's|^|generated_svsrc/|g' | tr '\n' ' ')

VERILATOR = verilator

CXXFLAGS += -CFLAGS "-DSPDLOG_USE_STD_FORMAT -O3 -I$(NPC_HOME)/include -std=c++23" -CFLAGS -DTOP_NAME="\"V$(TOPNAME)\"" -LDFLAGS "-lSDL2"
VERILATORFLAGS += --cc --exe --build --trace -j 16 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET


-include scripts/*.mk

npc: $(SV_FILELIST) $(LIBCAPSTONE) $(SO_PATH_NEMU)
	$(VERILATOR) $(VERILATORFLAGS) \
	--top-module $(TOPNAME) \
	$(CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) $(VERILATOR_DISABLE_WARNINGS) 

sbt: $(SV_FILELIST)

$(SV_FILELIST): $(CHISELSRC)
	sbt run 



PROG ?= test_progs/matrix-mul-riscv32e-npc.bin
ELF ?= test_progs/matrix-mul-riscv32e-npc.elf
MEM_SIZE ?= 134217728
TIME_STAMP ?= $(shell date +"%Y_%m_%d_%H_%M_%S")
LOG_FILE ?= "log/$(TIME_STAMP).log"
sim: npc
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) --itracer 8 --elf --difftest --inst_ringbuffer 16 $(NPC_FLAGS) 

sim_full_speed: npc
	$(call git_commit, "sim RTL full speed") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS) --enable_audio --enable_vga --enable_keyboard

sim_ftracer: npc
	$(call git_commit, "sim RTL ftracer") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --elf $(ELF) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS)


clean:
	rm -rf obj_dir
	sbt clean
	rm -rf generated_svsrc/*

clean_log:
	rm -rf log/*


.PHONY: npc sim clean sbt

include ../Makefile
