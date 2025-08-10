#!/usr/bin/env bash

echo -e "n\ne\nw\ne\nw\ne\n" | ./p19 | tee output1.txt
echo -e "l\nn\ne\na\nw\ne\na\nw\ne\nw\ne\na\n" | ./p19 | tee output2.txt