CSRCS = $(wildcard $(SOC_EMU_HOME)/src/*.cpp)
INC = $(SOC_EMU_HOME)/include

OPTIMFLAGS = -Ofast   \
-march=native -mtune=native \
-funroll-loops -flto=auto -ffast-math -fno-math-errno \
-funsafe-math-optimizations

CXXSTD = -std=c++23

SPDLOG_FLAGS = -DSPDLOG_USE_STD_FORMAT

EMU_CXXFLAGS = -I$(INC) $(SPDLOG_FLAGS) $(CXXSTD) $(OPTIMFLAGS) -fsanitize=address

CC = g++

MDIR = $(SOC_EMU_HOME)/build

$(MDIR):
	mkdir -p $@ || true

build: $(MDIR) $(CSRCS)
	$(CC) $(EMU_CXXFLAGS) $(CSRCS) -o $(MDIR)/soc_emu 

BIN ?= $(SOC_EMU_HOME)/images/microbench.bin

sim: build
	$(MDIR)/soc_emu --image $(BIN) --nr_cacheline 4 --nr_cachesize 0

clean:
	rm -rf build

.PHONY: build clean sim
