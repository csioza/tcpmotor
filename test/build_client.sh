#!/bin/bash

rm -rf CMakeLists.txt
cp CMakeLists.client.txt CMakeLists.txt

rm -rf *.h
cp ../*.h ./
cp ../history/concurrentqueue.h ./

rm -rf obj
mkdir obj
cd obj

cmake ..
make

cp client ../client