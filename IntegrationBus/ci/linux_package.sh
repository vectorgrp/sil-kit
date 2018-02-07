#!/bin/bash

buildNumber=$1
buildTargets=$2
buildFolders=$3
packageFolders=$4
componentGroups=$5

stop() {
    exitCode=$?;
    { set +x; } 2>&-
    echo "Fatal Error: Last build step failed with exit code $exitCode";
    exit "$exitCode";
}
pass() {
    exitCode=$?;
    { set +x; } 2>&-
    echo "Warning: Last build step failed with exit code $exitCode";
}

echo "Creating Linux packages"
echo "---------------------------------------------------------------------------------------------------"
echo "Build number: $buildNumber"
echo "Build targets: $buildTargets"
echo "Build folders: $buildFolders"
echo "Package folders: $packageFolders"
echo "Component groups: $componentGroups"

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Preparing folders..."

# We assume that we are already invoked from one of the buildFolders
# Remember where we are, this is where the packages should be placed
workingFolder=${PWD##*/}
echo "Working folder: $workingFolder"

# Create a clean subfolder 'package', so the caller can safely assume that none of the packages are stale
rm -rf package
mkdir package || stop
cd package || stop

IFS=";", read -ra buildTargetsArray <<< "$buildTargets"
IFS=";", read -ra buildFoldersArray <<< "$buildFolders"

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Configuring ${#buildFoldersArray[@]} projects '$buildFolders' for new docker mount point and packaging..."
for ((i=0;i<${#buildFoldersArray[@]};++i))
do
    buildFolder=${buildFoldersArray[$i]}
    target=${buildTargetsArray[$i]}

    mkdir $buildFolder
    cd $buildFolder || stop

    echo "Configuring project '$buildFolder' for new docker mount point and packaging..."
    set -x
    # Deleting the cache is crucial, CMake/CPack fail/hang when the Docker mount point is different to linux_build.sh
    rm CMakeCache.txt
    cmake .. \
        -DCMAKE_BUILD_TYPE=$target \
        -DIB_BUILD_NUMBER=$buildNumber \
        -DIB_INSTALL_PDB_FILES=OFF \
        -DCMAKE_INSTALL_PREFIX=../install \
        -DCMAKE_PREFIX_PATH=./install \
        -DCMAKE_INSTALL_SYMLINKS=OFF \
        -DCPACK_MULTICONFIG_PACKAGE=ON \
        -DCPACK_MULTICONFIG_BUILDFOLDERS=$buildFolders \
        -DCPACK_MULTICONFIG_PACKAGEFOLDERS=$packageFolders \
        || stop
    { set +x; } 2>&-

    echo
    echo "---------------------------------------------------------------------------------------------------"
    echo "Building project '$buildFolder'..."
    set -x
    cmake --build . || stop
    { set +x; } 2>&-

    # echo
    # echo "---------------------------------------------------------------------------------------------------"
    # echo "Running tests on project '$buildFolder'..."
    # set -x
    # ctest -C $target -VV -R "^Test" || stop
    # { set +x; } 2>&-
done

# Change to previously created output folder
cd ../../$workingFolder/package || stop

echo
echo "---------------------------------------------------------------------------------------------------"
echo "Packaging all component groups '$componentGroups' of project..."
IFS=";"
for componentGroup in $componentGroups
do
    echo "Packaging component group '$componentGroup' of project..."
    set -x
    cpack --config ../build/CPackConfig.cmake --verbose \
        -D CPACK_COMPONENT_GROUP=$componentGroup \
        || stop
    { set +x; } 2>&-
done

cd ..

echo "---------------------------------------------------------------------------------------------------"
echo "Packaging succeeded"
