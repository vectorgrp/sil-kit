#!/bin/sh
# fetch prebuilt binary boost 1.67.0 exported from subversion
# BinLog is linked against this file
set -u #bail on undefined var
boost_src="${ARTIFACTORY}/${SILKIT_ARTIFACTORY_REPO}/ThirdParty/boost-1.67.0-ubuntu18.04.tgz"
sha256=86352332734b31b2515de35aa2a270901efec41bea21f17779c3c3f490ff2b58
die() {
	echo -e --  "ERRROR $@"
	exit 2
}

if [ -e /opt/vector/include/boost/version.hpp ]
then
    vers="$(awk '/#define.*BOOST_LIB_VERS.*/{gsub(/"/,"", $3); print $3}' //opt/vector/include/boost/version.hpp)"
    if [ "$vers" == "1_67" ]
    then
        echo "Boost installation found"
        exit
    fi
fi

export NO_PROXY=${PROXY_DOMAIN}
export no_proxy=${PROXY_DOMAIN}

cd /tmp
tgz="$(basename ${boost_src})"
if [ ! -e "${tgz}" ]
then
    wget --no-check-certificate -O "${tgz}" "${boost_src}"  || (
        rm "${tgz}"
        die "Download of ${boost_src} failed"
    )
    _sum=$(sha256sum "${tgz}" | awk '{print $1}')
    if [ "${_sum}" != "${sha256}" ]
    then
        pwd
        ls -lahF .
        sha256sum *
        rm -f ${tgz}
        die "Downloaded cmake checksum mismatch: \nfile: ${tgz}\nwant: ${sha256}\nhave: ${_sum}"
    fi
fi
tar -xzf "${tgz}" -C /
