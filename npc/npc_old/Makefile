.DEFAULT_GOAL := sim

CSRC = $(shell find csrc -name "*.cpp")
CSRC += /usr/local/lib/libreplxx.a

VSRC = $(wildcard vsrc/*.v)
VSRC += $(wildcard vsrc/*.sv)
CHISELSRC += $(wildcard src/main/scala/empty/*.scala)
TOPNAME = npc_top
BIN = $(MDIR)/V$(TOPNAME)

SV_FILELIST=generated_svsrc/filelist.f

SVSRC_FILES = $(shell cat $(SV_FILELIST))
SVSRC = $(patsubst %,generated_svsrc/%,$(SVSRC_FILES))

VERILATOR = verilator

VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -Ofast \
-march=native -mtune=native \
-funroll-loops -flto=auto -ffast-math -fno-math-errno -funsafe-math-optimizations 
VERILATOR_LDFLAGS = -flto=auto -fipa-pta 


V_CXXFLAGS += -CFLAGS "$(VERILATOR_CFLAGS) -std=c++23 -I$(NPC_HOME)/include"  -LDFLAGS "-lSDL2" 
# VERILATORFLAGS += --noassert --trace --no-coverage --x-assign fast --x-initial fast -O3 --threads 1 --cc --exe --build -j 8 
VERILATORFLAGS += --trace -O3 --threads 1 --cc --exe --build -j 8 '-Dlayer$$Verifying' '-Dlayer$$AXIAssertLayer'
VERILATOR_DISABLE_WARNINGS += -Wno-DECLFILENAME
# VERILATOR_DISABLE_WARNINGS += -Wno-UNUSEDSIGNAL -Wno-DECLFILENAME -Wno-SYNCASYNCNET -Wno-PINCONNECTEMPTY

FAST_FLAGS = -Ofast -march=native -mtune=native

-include scripts/*.mk

MDIR = $(NPC_HOME)/build/default/

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
	sbt "run --init-pc 0x80000000 --to-npc" 
# 	bash -c "find generated_svsrc/ -name *.sv -exec sed -i 's+src/main/scala/empty/++g' {} \\;"



PROG ?= test_progs/recursion.bin
ELF  ?= test_progs/recursion.elf
MEM_SIZE ?= 0x8000000
MEM_BASE ?= 0x80000000
DEV_SIZE ?= 0x10000000
DEV_BASE ?= 0xa0000000
TIME_STAMP := $(shell date +"%Y_%m_%d_%H_%M_%S")
LOG_FILE ?= "log/$(TIME_STAMP).log"



sim: npc
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) \
	--mem_size $(MEM_SIZE) --mem_base $(MEM_BASE) \
	--device_size $(DEV_SIZE) --device_base $(DEV_BASE) \
	--log $(LOG_FILE) --difftest --enable_vga --enable_audio $(NPC_FLAGS) --itracer 8

sim_full_speed: VERILATOR_CFLAGS+=-DNDEBUG -fstrict-aliasing \
-frename-registers -fomit-frame-pointer \
-DDISABLE_ALL_TRACER -DNO_DIFFTEST
sim_full_speed: VERILATOR_LDFLAGS+=-fuse-linker-plugin
sim_full_speed: MDIR = $(NPC_HOME)/build/full_speed/
sim_full_speed: npc
	$(call git_commit, "sim RTL full speed") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --image $(PROG) \
	--mem_size $(MEM_SIZE) --mem_base $(MEM_BASE) \
	--device_size $(DEV_SIZE) --device_base $(DEV_BASE) \
	--log $(LOG_FILE) --difftest --enable_vga --enable_audio $(NPC_FLAGS) --batch

sim_ftracer: npc
	$(call git_commit, "sim RTL ftracer") # DO NOT REMOVE THIS LINE!!!
	$(BIN) --ftracer $(ELF) --itracer 8 --image $(PROG) --mem_size $(MEM_SIZE) --log $(LOG_FILE) $(NPC_FLAGS) --difftest

valgrind: FAST_FLAGS = -O0 -g
valgrind: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O0 -g -flto=auto
valgrind: MDIR = $(NPC_HOME)/build/valgrind/
valgrind: npc
	valgrind --leak-check=full --show-leak-kinds=all \
	$(BIN) --image $(PROG) \
	--mem_size $(MEM_SIZE) --mem_base $(MEM_BASE) \
	--device_size $(DEV_SIZE) --device_base $(DEV_BASE) \
	--log $(LOG_FILE) --ftracer $(ELF) --itracer 8


gdb: FAST_FLAGS = -O0 -g
gdb: VERILATOR_CFLAGS = -DSPDLOG_USE_STD_FORMAT -O0 -g
gdb: MDIR = $(NPC_HOME)/build/gdb/
gdb: npc
	gdb --args $(BIN) --image $(PROG) \
	--mem_size $(MEM_SIZE) --mem_base $(MEM_BASE) \
	--device_size $(DEV_SIZE) --device_base $(DEV_BASE) \
	--log $(LOG_FILE) --difftest


clean:
	rm -rf build
	sbt clean
	rm -rf generated_svsrc/*

clean_log:
	rm -rf log/*


.PHONY: npc sim sim_full_speed sim_ftracer clean sbt gdb valgrind

include ../Makefile
