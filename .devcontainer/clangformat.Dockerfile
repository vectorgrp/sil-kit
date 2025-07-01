FROM ubuntu:24.04
ENV TZ "Europe/Berlin"
ENV DEBIAN_FRONTEND "noninteractive"
ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

RUN \
        apt-get update \
        && \
        apt-get install -y \
            git \
            wget \
            gnupg2 \
            software-properties-common \
            lsb-base \
            lsb-release \
            clang-format \
            python3 \
        && \
        apt-get clean
WORKDIR /workspace