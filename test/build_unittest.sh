#!/bin/bash

#rm unittest

cd googletest-release-1.8.1
rm -rf obj
mkdir obj

cd obj

cmake ..
make

cd ..
cd ..

rm -rf CMakeLists.txt
cp CMakeLists.unittest.txt CMakeLists.txt
rm -rf *.h
cp ../*.h ./
rm -rf obj
mkdir obj
cd obj
cmake ..
make

mv unittest ../unittest