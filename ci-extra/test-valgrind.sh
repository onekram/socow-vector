#!/bin/bash
set -euo pipefail

BUILD_TYPE=$1

SCRIPT_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

valgrind --tool=memcheck \
  --gen-suppressions=all \
  --leak-check=full \
  --show-leak-kinds=all \
  --leak-resolution=med \
  --track-origins=yes \
  --vgdb=no \
  --error-exitcode=1 \
  --suppressions="${SCRIPT_DIR}/valgrind.suppressions" \
  cmake-build-"$BUILD_TYPE"/tests
