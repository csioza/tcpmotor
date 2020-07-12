#!/bin/bash

# sed -i "s/tcpmotor/server/g" "./CMakeLists.txt"
# sed -i "s/client/server/g" "./CMakeLists.txt"

# sed -i "s/TEST_CPP/test_server.cpp/g" "./CMakeLists.txt"
# sed -i "s/test_client.cpp/test_server.cpp/g" "./CMakeLists.txt"

rm -rf CMakeLists.txt
cp CMakeLists.server.txt CMakeLists.txt

rm -rf *.h
cp ../*.h ./
cp ../history/concurrentqueue.h ./

rm -rf obj
mkdir obj
cd obj

cmake ..
make

cp server ../server