#!/bin/bash

set -euxo pipefail

# ./gimp/rgba_headers.py ./gimp/*.png --output_dir include

[[ -d build ]] || mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=on # -DUSE_ASAN=on
make -j
