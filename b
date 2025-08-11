#!/bin/bash

set -euxo pipefail

[[ -d build ]] || mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DUSE_ASAN=on
make -j
