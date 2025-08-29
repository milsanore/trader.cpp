#!/usr/bin/env bash

[ -d "build" ] && rm -r "build"
mkdir -p build
cd build
cmake ..
cmake --build .
./tradercpp
