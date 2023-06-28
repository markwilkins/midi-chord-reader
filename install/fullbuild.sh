#!/bin/bash
#

# store current folder
pushd .

# do release build
mkdir -p ../release
cd ../release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
ctest -j50 --output-on-failure

# return to install folder
popd
./buildinst.sh
