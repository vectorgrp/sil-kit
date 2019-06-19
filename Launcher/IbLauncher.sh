#!/bin/bash

workingDir=$(dirname "$(realpath -s "$0")")

# Hint at local installation
export INTEGRATIONBUS_BINPATH="$workingDir"
export INTEGRATIONBUS_LIBPATH="$workingDir/../lib"

python $workingDir/../share/doc/IntegrationBus-Launcher/IbLauncher.py "$@"
