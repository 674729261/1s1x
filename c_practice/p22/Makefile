CFLAGS=-Wall -g -DNDEBUG -O0

all: p22_main stackoverflow
	@echo "练习22：栈、作用域和全局"
	@read e
	./p22_main
	./stackoverflow 114 514 || true

p22_main: p22.o

clean:
	rm -f p22_main p22.o stackoverflow