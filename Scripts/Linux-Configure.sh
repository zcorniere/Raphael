#! /bin/bash

set -euo pipefail

export CC=clang
export CXX=clang++

BUILD_LOCATION="$(pwd)/build"

# Parse  arguments
# --profiling
# --memory-profiling
# --no-nan-check
# --no-rendering-debugging
# --asan

PROFILING=0
MEMORY_PROFILING=0
NAN_CHECK=1
RENDERING_DEBUGGING=1
ENABLE_ASAN=0
BUILD_TYPE="Debug"
while [[ $# -gt 0 ]]; do
    case $1 in
    --profiling)
        PROFILING=1
        shift 1
        ;;
    --memory-profiling)
        PROFILING=1
        MEMORY_PROFILING=1
        shift 1
        ;;
    --no-nan-check)
        NAN_CHECK=0
        shift 1
        ;;
    --no-rendering-debugging)
        RENDERING_DEBUGGING=0
        shift 1
        ;;
    --asan)
        ENABLE_ASAN=1
        shift 1
        ;;
    --build-type)
        BUILD_TYPE=$2
        shift 2
        ;;
    *)
        echo "Unknown argument: $1"
        exit 1
        ;;
    esac
done

# Check if the build type is valid
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "RelWithDebInfo" && "$BUILD_TYPE" != "MinSizeRel" ]]; then
    echo "Invalid build type: $BUILD_TYPE"
    exit 1
fi

mkdir -vp "$BUILD_LOCATION"
cd "$BUILD_LOCATION"

cmake -G "Ninja Multi-Config" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DRPH_ENABLE_PROFILING=$PROFILING \
    -DRPH_ENABLE_MEMORY_PROFILING=$MEMORY_PROFILING \
    -DRPH_NAN_CHECKS=$NAN_CHECK \
    -DRPH_ENABLE_VULKAN_DEBUGGING=$RENDERING_DEBUGGING \
    -DRPH_ENABLE_ASAN=$ENABLE_ASAN \
    ..

ln -svf "$BUILD_LOCATION/compile_commands.json" ../compile_commands.json
