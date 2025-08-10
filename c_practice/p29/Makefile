CFLAGS=-fPIC -DNDEBUG -O2 -Iinc
OBJECT=src/lib.o
SO=build/lib.so
TEST=test/p29

all: build $(SO) test/p29
	@echo "练习29：库和链接"
	@read e
	bash -e test/autotest.sh

$(SO): $(OBJECT)
	$(CC) -fPIC -shared $(OBJECT) -o $@

$(OBJECT): $(CFLAGS)+=-c

autotest:
	bash -e test/autotest.sh

build:
	mkdir -p build bin

clean:
	rm -rf build bin src/*.o test/p29 test/output.txt

