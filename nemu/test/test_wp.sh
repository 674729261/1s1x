make
cat ./test/test_wp.tcl | ./build/riscv32-nemu-interpreter --log=./build/nemu-log.txt ./test/test_img.bin
TIME1=$(cat ./build/nemu-log.txt | grep -o -P "\d+(?= inst/s)")
echo -e "c\nq" | ./build/riscv32-nemu-interpreter --log=./build/nemu-log.txt ./test/test_img.bin
TIME2=$(cat ./build/nemu-log.txt | grep -o -P "\d+(?= inst/s)")
echo "无监视点时速度为${TIME2} inst/s"
echo "有监视点时速度为${TIME1} inst/s, 是无监视点时的0.$(($TIME1 * 100 / $TIME2))倍"