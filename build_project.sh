#!/bin/bash

# Cleaning build directory
echo "Cleaning build directory..."
rm -rf build
mkdir -p build
cd build || exit 1

# Configuring with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Building project
echo "Building project..."
cmake --build . -- -j$(nproc)

# Build completed
echo "Build completed."