make
cat ./test/test_wp.tcl | ./build/riscv32-nemu-interpreter --log=./build/nemu-log.txt ./test/test_img.bin
TIME1=$(cat ./build/nemu-log.txt | grep -o -P "\d+(?= inst/s)")

echo "速度为${TIME1} inst/s"