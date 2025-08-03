all:
# 	@echo "Write this Makefile by your self."
	verilator --cc --exe --build -j 0 -Wall csrc/light_sim.cpp vsrc/light.v

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# 	@echo "Write this Makefile by your self."
	./obj_dir/Vlight
	
clean:
	rm -rf obj_dir compile_commands.json .cache


include ../Makefile
