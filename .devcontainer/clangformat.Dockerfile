FROM ghcr.io/vectorgrp/sil-kit-docker-build/sil-kit-ci-public-runner:main

WORKDIR /workspace

RUN ./Silkit/ci/check_formatting.py --force-formatting
