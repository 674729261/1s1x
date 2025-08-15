CFLAGS=-Wall -g
all: p01 p03 p04 p05 p06 p07 p08 p09 p10 p11 p12 p13 p13_2 p14 p15 p15_pointer p16 p16_2 p17 p17_2 p17_stack p18 p18_2 show_version
	make -C p19
	make -C p20
	make -C p21
	make -C p22
	make -C p24
	make -C p25
	make -C p29
	make -C p31
	make -C p32_33
	make -C p42
	make -C p44



clean:
	rm -f p01 p03 p04 p05 p06 p07 p08 p09 p10 p11 p12 p13 p13_2 p14 p15 p15_pointer p16 p16_2 p17 p17_2 p17_stack p18 p18_2 
	make -C p19 clean
	make -C p20 clean
	make -C p21 clean
	make -C p22 clean
	make -C p24 clean
	make -C p25 clean
	make -C p29 clean
	make -C p31 clean
	make -C p32_33 clean
	make -C p42 clean
	make -C p44 clean

.PHONY: show_version

show_version:
	@$(CC) --version