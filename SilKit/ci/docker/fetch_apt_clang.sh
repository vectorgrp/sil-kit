#!/bin/sh
set -e
set -u

# add official clang builds
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
if [ "${UBUNTU_VERSION}" = "22.04" ]
then 

    cat > /etc/apt/sources.list.d/clang-apt.list <<EOF
# 15
deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-15 main
deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-15 main
# 16
deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main
deb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main
EOF

elif [ "${UBUNTU_VERSION}" = "20.04" ]
then
    cat > /etc/apt/sources.list.d/clang-apt.list <<EOF
# 15
deb http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main
deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-15 main
# 16
deb http://apt.llvm.org/focal/ llvm-toolchain-focal-16 main
deb-src http://apt.llvm.org/focal/ llvm-toolchain-focal-16 main
EOF

elif [ "${UBUNTU_VERSION}" = "18.04" ]
then
    cat > /etc/apt/sources.list.d/clang-apt.list <<EOF
# Needs 'sudo add-apt-repository ppa:ubuntu-toolchain-r/test' for libstdc++ with C++20 support
# 15
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-15 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-15 main
# 16
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-16 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-16 main
EOF

fi
echo "Ubuntu ${UBUNTU_VERSION}: added clang apt repo"

if [ "${UBUNTU_VERSION}" = "22.04" ]
then
    echo "Ubuntu ${UBUNTU_VERSION}: Installing clang-14"
    apt-get install --no-install-recommends --no-install-suggests -y \
        clang-14 llvm-14
elif [ "${UBUNTU_VERSION}" = "20.04" ]
then 
    echo "Ubuntu ${UBUNTU_VERSION}: Installing clang-12 gcc-10";
    apt-get install --no-install-recommends --no-install-suggests -y \
        clang-15 llvm-15 \
        clang-12 llvm-12 \
        gcc-10 g++-10 
else 
    echo "Ubuntu ${UBUNTU_VERSION}: Installing clang-8 gcc-8"
    apt-get install --no-install-recommends --no-install-suggests -y \
        clang-8 llvm-8 \
        gcc-8 g++-8 
fi
