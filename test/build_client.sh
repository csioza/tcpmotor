#!/bin/bash

sed -i "s/tcpmotor/client/g" "./CMakeLists.txt"
sed -i "s/server/client/g" "./CMakeLists.txt"

sed -i "s/TEST_CPP/test_client.cpp/g" "./CMakeLists.txt"
sed -i "s/test_server.cpp/test_client.cpp/g" "./CMakeLists.txt"

rm -rf obj

mkdir obj

cd obj

cmake ..
make