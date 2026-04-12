#!/bin/bash

# By default, exit on error. However, some tests expect the build to fail, these
# tests will temporarily disable this behavior.
set -e

BUILD_DIR="build/build-tests"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color (reset)

print_test_result() {
    local result=$1
    local condition="$2"   # Should be "-eq" (expect success) or "-ne" (expect failure)
    local pass_msg=$3
    local fail_msg=$4

    # If fail_msg is empty, use pass_msg instead
    if [ -z "$fail_msg" ]; then
        fail_msg="$pass_msg"
    fi

    if [ $result $condition 0 ]; then
        echo -e "${GREEN}[PASSED] build test: ${pass_msg}${NC}"
    else
        set -e
        echo -e "${RED}[FAILED] build test: ${fail_msg}${NC}"
        exit 1
    fi
}

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
cmake -G Ninja ../.. && cmake --build .
EXIT_CODE=$?
cd ../..
print_test_result $EXIT_CODE -eq "default"

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
cmake -G Ninja -DSTATIC_ONLY=True ../.. &&  cmake --build .
EXIT_CODE=$?
cd ../..
print_test_result $EXIT_CODE -eq "static-only"

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
cmake -G Ninja -DSHARED_ONLY=True ../.. &&  cmake --build .
EXIT_CODE=$?
cd ../..
print_test_result $EXIT_CODE -eq "shared-only"

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
set +e
cmake -G Ninja -DSHARED_ONLY=True -DSTATIC_ONLY=True ../..
EXIT_CODE=$?
set -e
cd ../..
print_test_result $EXIT_CODE -ne "shared-only and static-only (should fail)"

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
cmake -G Ninja -DDEFAULTS=True ../.. &&  cmake --build .
EXIT_CODE=$?
cd ../..
print_test_result $EXIT_CODE -eq "defaults"

rm -rf "${BUILD_DIR}" && mkdir -p "${BUILD_DIR}" && cd "${BUILD_DIR}"
cmake -G Ninja ../../test/cmake_externalproject &&  cmake --build .
EXIT_CODE=$?
cd ../..
print_test_result $EXIT_CODE -eq "tests super build"

# Clean up.
# rm -rf "${BUILD_DIR}"
