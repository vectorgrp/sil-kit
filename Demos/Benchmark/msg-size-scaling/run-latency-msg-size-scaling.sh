#!/bin/sh

# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

#Usage: ./run-latency-msg-size-scaling.sh <path/to/SilKitDemoLatency> <path/to/result.csv> [<path/to/SilKitConfig>]

EXE=$1
CSVFILE=$2
CONFIGFILE=${3:-}
CONFIGARG=""
if [ ! -z "${CONFIGFILE}" ]; then
    CONFIGARG="--configuration ${CONFIGFILE}"
fi

MSGCOUNT=10000

for msgsize in 10 100 1000 2000 5000 10000 20000 50000 100000 1000000 10000000 100000000
do
  echo "Run SilKitLatencyDemo with MSGCOUNT=${MSGCOUNT}, MSGSIZE=${msgsize}B, CONFIG=${CONFIGFILE}, CSV=${CSVFILE}"

  $EXE --message-count ${MSGCOUNT} \
       --message-size ${msgsize} \
       --isReceiver \
       ${CONFIGARG} > /dev/null &
       
  $EXE --message-count ${MSGCOUNT} \
       --message-size ${msgsize} \
       --write-csv ${CSVFILE} \
       ${CONFIGARG} > /dev/null

done
