#!/bin/bash
set -euo pipefail

BUILD_TYPE=$1

# Configure CMake
cmake -S . -B cmake-build-"$BUILD_TYPE" \
  --preset "$BUILD_TYPE" -G Ninja \
  -DTREAT_WARNINGS_AS_ERRORS=ON

# Build
cmake --build cmake-build-"$BUILD_TYPE" -j
