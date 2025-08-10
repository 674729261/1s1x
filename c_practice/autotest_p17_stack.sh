set -e
./p17_stack data/stack_db.dat c 128
./p17_stack data/stack_db.dat p 33
./p17_stack data/stack_db.dat p 12
./p17_stack data/stack_db.dat p 45
./p17_stack data/stack_db.dat q
./p17_stack data/stack_db.dat p 67
./p17_stack data/stack_db.dat p 23
./p17_stack data/stack_db.dat q
./p17_stack data/stack_db.dat q
./p17_stack data/stack_db.dat p 87
./p17_stack data/stack_db.dat p 23
./p17_stack data/stack_db.dat q
./p17_stack data/stack_db.dat p 76
./p17_stack data/stack_db.dat p 87
./p17_stack data/stack_db.dat l > output.txt
vimdiff output.txt answer_p17_stack.txt
rm data/stack_db.dat output.txt
