all:
# 	@echo "Write this Makefile by your self."
	verilator --cc --exe --build -j 0 -Wall csrc/main.cpp vsrc/top_a.v

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# 	@echo "Write this Makefile by your self."
	./obj_dir/Vtop_a
	
clean:
	rm -rf obj_dir compile_commands.json .cache


include ../Makefile
