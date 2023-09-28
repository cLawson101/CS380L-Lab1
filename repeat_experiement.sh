#!/bin/sh
echo "Starting Tests"

g++ -o binary lab1_io_code.cpp

fincore file-1g

for t in 1 2 3 4 5 6 7 8 9 10
do
	./binary
done
