#!/bin/sh
# Same build environment as in JenkinsFile for reproducibility.
# NB: currently we rely on binary FastRTPS packages on our CI, these
#     are required to reproduce the binaries created by our CI environment.
# You can override REPO, SOURCE and BUILD on the command line, e.g.
#    $ BUILD=my_repro_builddir ./repro.sh
set -e
set -u
export SOURCE_DATE_EPOCH=$(git log --max-count=1  --format=%ct -r origin/master)
export LC_ALL=C
export TZ=UTC
# XXX download / copy fastrtps*.zip into $REPO manually
REPO=${REPO:-file:///${PWD}/distrib}
SOURCE=${SOURCE:-.}
BUILD=${BUILD:-_build_repro}

cmake -S $SOURCE -B ${BUILD} -G Ninja -D CMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Release -DIB_INSTALL_SOURCE=ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++

cmake --build ${BUILD} --target package $@
