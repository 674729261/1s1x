.DEFAULT_GOAL := sim

CSRC = $(shell find csrc -name "*.cpp")
CSRC += /usr/local/lib/libreplxx.a

VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = CPU
BIN = ./obj_dir/V$(TOPNAME)

SV_FILELIST=generated_svsrc/filelist.f

SVSRC_FILES := $(shell cat $(SV_FILELIST))
SVSRC := $(patsubst %,generated_svsrc/%,$(SVSRC_FILES))

VERILATOR = verilator

VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -Ofast   \
-march=native -mtune=native \
-funroll-loops -flto=auto -ffast-math -fno-math-errno -funsafe-math-optimizations 
VERILATOR_LDLAGS = -flto=auto -fipa-pta 


CXXFLAGS += -CFLAGS "$(VERILATOR_CFLAGS) -std=c++23 -I$(NPC_HOME)/include"  -LDFLAGS "-lSDL2" 
VERILATORFLAGS += --noassert --no-coverage --x-assign fast --x-initial fast -O3 --no-trace --threads 1 --cc --exe --build -j 8 
VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET -Wno-PINCONNECTEMPTY


FAST_FLAGS = -Ofast -march=native -mtune=native

-include scripts/*.mk

npc: $(SV_FILELIST) $(LIBCAPSTONE) $(SO_PATH_NEMU)
	$(VERILATOR)  $(VERILATORFLAGS) \
	--top-module $(TOPNAME)  \
	-CFLAGS -DTOP_NAME="\"V$(TOPNAME)\"" \
	--MAKEFLAGS "-j12 OPT_FAST=\"$(FAST_FLAGS)\"" \
	$(CXXFLAGS) -Wall $(CSRC) $(SVSRC) $(VSRC) $(VERILATOR_DISABLE_WARNINGS) 

sbt: $(SV_FILELIST)

$(SV_FILELIST): $(CHISELSRC)
	sbt run 
# 	bash -c "find generated_svsrc/ -name *.sv -exec sed -i 's+src/main/scala/empty/++g' {} \\;"



PROG ?= test_progs/typing.bin
ELF ?= test_progs/typing.elf
MEM_SIZE ?= 134217728
TIME_STAMP := $(shell date +"%Y_%m_%d_%H_%M_%S")
LOG_FILE ?= "log/$(TIME_STAMP).log"
sim: npc
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) --itracer 8 --difftest --inst_ringbuffer 16 $(NPC_FLAGS) --enable_audio --enable_vga --enable_keyboard 

sim_full_speed: VERILATOR_CFLAGS+=-DNDEBUG -fstrict-aliasing \
-frename-registers -fomit-frame-pointer \
-DDISABLE_ALL_TRACER -DNO_DIFFTEST
sim_full_speed: VERILATOR_LDLAGS+=-fuse-linker-plugin
sim_full_speed: npc
	$(call git_commit, "sim RTL full speed") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS) --enable_audio --enable_vga --enable_keyboard

sim_ftracer: npc
	$(call git_commit, "sim RTL ftracer") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --elf $(ELF) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS) --difftest

sim_swap: VERILATOR_CFLAGS+=-DNDEBUG -fstrict-aliasing \
-frename-registers -fomit-frame-pointer \
-DDISABLE_ALL_TRACER -DNO_DIFFTEST
sim_swap: VERILATOR_LDLAGS+=-fuse-linker-plugin
sim_swap: npc
	$(call git_commit, "sim swap") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS) --enable_audio --enable_vga --enable_keyboard

valgrind: PROG = test_progs/add.bin
valgrind: ELF = test_progs/add.elf
valgrind: FAST_FLAGS = -O0 -g
valgrind: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O0 -g
valgrind: npc
	valgrind --leak-check=full --show-leak-kinds=all \
	 ./obj_dir/VCPU --image $(PROG) --mem_size 134217728 --log $(LOG_FILE) --itracer 8 --inst_ringbuffer 16 --elf $(ELF)  --batch 



gdb: FAST_FLAGS = -O3 -g
gdb: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O3 -g
gdb: npc
	gdb --args ./obj_dir/VCPU --image $(PROG) --mem_size 134217728 --log $(LOG_FILE) --itracer 8 --difftest --inst_ringbuffer 16 


clean:
	rm -rf obj_dir
	sbt clean
	rm -rf generated_svsrc/*

clean_log:
	rm -rf log/*


.PHONY: npc sim sim_full_speed sim_ftracer clean sbt gdb valgrind

include ../Makefile
