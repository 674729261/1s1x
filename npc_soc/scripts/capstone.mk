LIBCAPSTONE = tools/capstone/repo/libcapstone.so.6
SO_PATH_CAPSTONE = $(SOC_NPC_HOME)/$(LIBCAPSTONE)
V_CXXFLAGS += -CFLAGS "-I$(SOC_NPC_HOME)/tools/capstone/repo/include -DSO_PATH_CAPSTONE=$(SO_PATH_CAPSTONE)"

$(LIBCAPSTONE):
	$(MAKE) -C tools/capstone