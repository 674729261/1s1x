build:
	verilator --cc --exe --build --trace -j 16 -Wall csrc/test.cpp vsrc/sw.v

run: build
	./obj_dir/Vsw

clean:
	rm -rf obj_dir