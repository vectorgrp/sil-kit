#!/bin/sh
# fetch an appropriate cmake if necessary
# we need at least cmake 3.11, ubuntu 18.04 LTS has 3.10
set -u #bail on undefined var
CMAKE_VERSION=${CMAKE_VERSION:-3.25.1}
dist="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz"
dest="/opt/vector"
sha256="3a5008b613eeb0724edeb3c15bf91d6ce518eb8eebc6ee758f89a0f4ff5d1fd6"
die() {
	echo -e --  "ERRROR $@"
	exit 2
}
#check that cmake has a minor number that is larger than 10 (e.g. 3.11.X)
if which cmake >/dev/null
then
    vers="$(cmake --version| awk '/^cmake v/{print $3}')"
    minor="$(echo $vers | sed -e 's;[0-9]\+\.\([0-9]\+\)\.[0-9]\+;\1;g')"
    if [ "${minor}" -gt "11" ]
    then
        echo "Suitable cmake found: version ${vers}"
        exit 0
    fi
fi
(
    set -e
    mkdir -p ${dest} 2> /dev/null
    cd /tmp
    tgz=$(basename ${dist})
    wget ${dist}
    _sum=$(sha256sum  ${tgz} | awk '{print $1}')
    if [ "${sha256}" != "${_sum}" ]
    then
        rm -f ${tgz}
        die "Downloaded cmake checksum mismatch:\nwant: ${sha256}\nhave: ${_sum}"
    fi
    tar -xzf ${tgz} --strip=1 -C ${dest}
) || die cmake fetch failed
