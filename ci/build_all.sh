#!/bin/sh
set -e
set -u
#PRO refers to the CMake target which will be build
PRO=IntegrationBus
#only set artifactory if not already set in env
ARTIFACTORY="${ARTIFACTORY:-https://vistrpndart1.vi.***VIB-820 Removed***/artifactory/***VIB-820 Removed***}"
#cmake extra args
cmake_args="-DIB_BIN_FASTRTPS_ENABLE=1 -DIB_BIN_FASTRTPS_REPOSITORY=${ARTIFACTORY}/ThirdParty/FastRTPS/"


log() {
    echo  "$@"
}
info() {
    log INFO $@
}
die() {
    log ERROR $@
    exit 2
}
DIST=$PWD/distrib
mkdir -p ${DIST}
case "$(uname -s)" in
Linux)
    echo "Building for Linux"
    for compiler in g++ clang++
    do
        #build docs only once per buildtype, to safe time
        doc_build="-DBUILD_DOCS=ON"
        for cfg in Release Debug
        do
            info Building ${compiler} ${cfg}
            b="_build_${compiler}_${cfg}"
            info Build dir: $b
            mkdir -p $b
            (
                cd $b
                cmake ../ -DCMAKE_CXX_COMPILER=${compiler} -DCMAKE_BUILD_TYPE=${cfg} ${cmake_args} ${doc_build}
                cmake --build . --target package --parallel 4
                mv ${PRO}*.zip ${DIST}
            )
            doc_build=""
        done
    done
    ;;
MINGW*)
    echo "Building for Windows!"
    for arch in "" " Win64"
    do
        #for VS2015 we use a VS2017 solution with the v140 toolset (-T v140)
        for compiler in  "15 2017" "15 2017;v140"
        do
            tools=""
            if echo $compiler | grep -q ';'
            then
                tools="-T ${compiler#*;}"
                compiler=${compiler%;*}
            fi
            gen="Visual Studio ${compiler}${arch}"
            info "Using Generator: $gen"
            b="_build_win_${arch}_${compiler}${tools}"
            b="$(echo -n $b| sed 's; ;_;g')"
            info Build dir: $b
            for cfg in Release Debug
            do
                mkdir -p "${b}"
                (
                    cd ${b}
                    cmake ../ -G "${gen}" ${tools} -DCMAKE_BUILD_TYPE=${cfg} ${cmake_args}
                    cmake --build . --target package --parallel 4 --config ${cfg}
                    mv ${PRO}*.zip ${DIST}
                )
            done
        done
    done
    ;;
esac
