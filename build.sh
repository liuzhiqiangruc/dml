#!/bin/bash
mkdir output1
mkdir build
cd build
cmake ..  -DCMAKE_INSTALL_PREFIX=../output1
make && make install && make clean
