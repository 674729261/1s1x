hierarchy -check -top counter
proc
opt
techmap
splitnets -ports
opt -full
read_liberty -lib cell.lib
dfflibmap -liberty cell.lib
abc -liberty cell.lib
show
write_verilog netlist.v
stat -liberty cell.lib