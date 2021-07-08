#!/bin/bash

# Change the directory to the location of the build script.
cd "$(dirname "$0")"

# Sets the build configuration.
build_type=$1
if [[ $# -eq 0 ]]; then
    echo "No build type selected"
    build_type=Debug
fi

cmake -B build -S . -DCMAKE_C_COMPILER=gcc-10 \
                    -DCMAKE_CXX_COMPILER=g++-10 \
                    -DCMAKE_BUILD_TYPE=$build_type \
                    -DCMAKE_TOOLCHAIN_FILE=/Users/wbuckley/vcpkg/scripts/buildsystems/vcpkg.cmake
result=$?
if [ $result -ne 0 ]; then
    echo "Cmake failed"
    exit 1
fi

cmake --build build
result=$?
if [ $result -ne 0 ]; then
    echo "Build failed"
    exit 1
fi