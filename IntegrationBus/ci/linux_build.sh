#!/bin/bash

target=$1
buildNumber=$2

stop() {
    exitCode=$?;
    { set +x; } 2>&-
    echo "Fatal Error: Last build step failed with exit code $exitCode";
    exit "$exitCode";
}
pass() {
    { set +x; } 2>&-
    exitCode=$?;
    echo "Warning: Last build step failed with exit code $exitCode";
}

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Preparing folders..."

mkdir build
cd build || stop

echo "Performing a Linux build"
echo "Target: $target"
echo "Build number: $buildNumber"

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Configuring project for '$target'..."
set -x
rm CMakeCache.txt
cmake .. \
    -DCMAKE_BUILD_TYPE=$target \
    -DIB_BUILD_NUMBER=$buildNumber \
    -DIB_INSTALL_PDB_FILES=ON \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DCMAKE_PREFIX_PATH=./install \
    -DCMAKE_INSTALL_SYMLINKS=OFF \
    -DCPACK_MULTICONFIG_PACKAGE=OFF \
    || stop
{ set +x; } 2>&-

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Building project..."
set -x
cmake --build . || stop
{ set +x; } 2>&-

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Running tests on project..."
set -x
ctest -C %target% -VV -R '^Test' || stop
{ set +x; } 2>&-

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Installing all components of project..."
set -x
cmake --build . --target install || stop
{ set +x; } 2>&-

echo "---------------------------------------------------------------------------------------------------"
echo "Build succeeded"
