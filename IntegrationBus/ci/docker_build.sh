#!/bin/bash

docker build . -t $1 -f $(dirname $0)/${1}/Dockerfile \
 --build-arg http_proxy=$HTTP_PROXY \
 --build-arg https_proxy=$HTTP_PROXY \
 --build-arg HTTP_PROXY=$HTTP_PROXY \
 --build-arg HTTPS_PROXY=$HTTP_PROXY
