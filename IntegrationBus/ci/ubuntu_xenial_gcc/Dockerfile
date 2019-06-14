# Start with Ubuntu:Xenial (16.04 LTS)
FROM ubuntu:xenial

# Set working directory
WORKDIR /repo

# Add jenkins user and give write permissions for /repo to everyone
RUN useradd -r -g users -u 500 jenkins &&\
    chmod a+w /repo

# Configure base image with the basic things we need and which are unlikely to change
# See https://docs.docker.com/develop/develop-images/dockerfile_best-practices/
RUN apt-get update -y &&\
    apt-get upgrade -y &&\
    apt-get dist-upgrade -y &&\
    # Install prerequisites for building, testing and packaging
    apt-get install -y \
        cmake \
        git \
        python \
        zip \
        doxygen \
        python3-breathe
# Install additional prerequisites on top of the base image
RUN apt-get install g++ -y

USER jenkins

# Default command, can be overwritten from command line on 'docker run ... <CMD>'
CMD ["/bin/bash", "./IntegrationBus/ci/linux_build.sh", "Debug"]
