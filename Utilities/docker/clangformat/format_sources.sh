#!/bin/sh
set -e
set -u
SILKIT_SOURCE_DIR=${SILKIT_SOURCE_DIR:-$(git rev-parse --show-toplevel)}
docker build -t sil-kit-dev
docker run --rm -it -v "$SILKIT_SOURCE_DIR":/workspace silkit-dev
