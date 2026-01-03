.PHONY: build clean

RVGCC=riscv64-linux-gnu-gcc
RVOBJCOPY=riscv64-linux-gnu-objcopy
RVOBJDUMP=riscv64-linux-gnu-objdump


CSRC=$(wildcard ./tests/*.c)

MDIR=./build

ELF=$(MDIR)/mromtest.elf
BIN=$(MDIR)/mromtest.bin
ASM=$(MDIR)/mromtest.txt


build: $(ELF) $(BIN) $(ASM)

clean:
	rm -rf $(MDIR)


$(ELF):$(CSRC)
	mkdir -p $(MDIR)
	$(RVGCC) $(CSRC) -march=rv32i -mabi=ilp32 \
	-nostdlib -nostartfiles -fno-stack-protector \
	-fno-pic -T link.ld \
	-I./include -O2 -o $@

$(BIN): $(ELF)
	$(RVOBJCOPY) -O binary $(ELF) $@

$(ASM): $(BIN)
	$(RVOBJDUMP) -d $(ELF) > $@
