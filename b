#!/bin/bash

set -euxo pipefail

# ./gimp/rgba_headers.py ./gimp/*.png --output_dir include/sprites

[[ -d build ]] || mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DUSE_ASAN=on
# cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=on
make -j
