CSRCS = $(wildcard $(SOC_EMU_HOME)/src/*.cpp)
INC = $(SOC_EMU_HOME)/include

OPTIMFLAGS = -Ofast   \
-march=native -mtune=native \
-funroll-loops -flto=auto -ffast-math -fno-math-errno \
-funsafe-math-optimizations

CXXSTD = -std=c++23

SPDLOG_FLAGS = -DSPDLOG_USE_STD_FORMAT

EMU_CXXFLAGS = -I$(INC) $(SPDLOG_FLAGS) $(CXXSTD) $(OPTIMFLAGS)

CC = g++

MDIR = $(SOC_EMU_HOME)/build

$(MDIR):
	mkdir -p $@ || true

OBJDIR := $(MDIR)/obj
OBJS := $(patsubst $(SOC_EMU_HOME)/src/%.cpp,$(OBJDIR)/%.o,$(CSRCS))

build: $(MDIR)/soc_emu

$(MDIR)/soc_emu: $(OBJS)
	$(CC) $(EMU_CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SOC_EMU_HOME)/src/%.cpp | $(OBJDIR)
	@echo "CC $(shell basename $<)"
	@$(CC) $(EMU_CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

TIME_STAMP := $(shell date +"%Y_%m_%d_%H_%M_%S")
BIN ?= $(SOC_EMU_HOME)/images/microbench_train.bin
LOG_DIR ?= $(SOC_EMU_HOME)/logs
LOG_FILE ?= $(LOG_DIR)/$(TIME_STAMP).log
CACHE_N ?= 3
CACHE_M ?= 2


$(LOG_DIR):
	mkdir -p $@ || true

sim: build $(LOG_DIR)
	$(MDIR)/soc_emu --image $(BIN) \
	--nr_cacheline $(CACHE_N) --nr_cachesize $(CACHE_M) --log $(LOG_FILE)

clean:
	rm -rf build

.PHONY: build clean sim
