CFLAGS=-O2 -g -I./

run: minirvEMU
	./minirvEMU ./prog/mem.data 10000 4632
	./minirvEMU ./prog/sum.data 10000 552

minirvEMU:
	$(CC) $(CFLAGS) -o minirvEMU minirvEMU.c

clean:
	rm -f minirvEMU *.o 