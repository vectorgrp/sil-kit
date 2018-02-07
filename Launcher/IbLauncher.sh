#!/bin/bash

workingDir="$(dirname "$0")"
python $workingDir/../share/doc/IntegrationBus-Launcher/IbLauncher.py "$@"

