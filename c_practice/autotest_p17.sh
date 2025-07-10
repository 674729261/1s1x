set -e
./p17_2 data/db.dat c 128 256
./p17_2 data/db.dat s 1 shitful stf@qq.com
./p17_2 data/db.dat s 12 qwert asdfg@zxcv.com
./p17_2 data/db.dat s 77 ____ ____@____.___
./p17_2 data/db.dat d 12
./p17_2 data/db.dat s 120 1s1x 1s1x@1s1x.com
./p17_2 data/db.dat s 12 shitful stf@1s1x.com
./p17_2 data/db.dat s 7 77 77@77.77
./p17_2 data/db.dat l > output.txt
 vimdiff output.txt answer_p17.txt
rm data/db.dat output.txt
