#!/bin/sh

# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

#Usage: ./performance-diff.sh <name-version-1> <path/to/SilKitDemoBenchmark1> <name-version-1> <path/to/SilKitDemoBenchmark2> <path/to/diff-result.csv>

NAME1=$1
EXE1=$2
NAME2=$3
EXE2=$4
CSVDIFF=$5

CSVRUN1="result-version-${NAME1}.csv"
rm ${CSVRUN1}
CSVRUN2="result-version-${NAME2}.csv"
rm ${CSVRUN2}

REPEAT=5000
SIMTIME=5
NUMPART=2
MSGCOUNT=1
MSGSIZE=100

echo "Run ${EXE1} with RUNS=${REPEAT}, T=${SIMTIME}s, PARTICIPANTS=${NUMPART}, MSGCOUNT=${MSGCOUNT}, MSGSIZE=${MSGSIZE}B, CONFIG=${CONFIGFILE}, CSV=${CSVRUN1}"
$EXE1 --number-simulation-runs ${REPEAT} \
      --simulation-duration ${SIMTIME} \
      --number-participants ${NUMPART} \
      --message-count ${MSGCOUNT} \
      --message-size ${MSGSIZE} \
      --write-csv ${CSVRUN1}

echo "Run ${EXE2} with RUNS=${REPEAT}, T=${SIMTIME}s, PARTICIPANTS=${NUMPART}, MSGCOUNT=${MSGCOUNT}, MSGSIZE=${MSGSIZE}B, CONFIG=${CONFIGFILE}, CSV=${CSVRUN2}"
$EXE2 --number-simulation-runs ${REPEAT} \
      --simulation-duration ${SIMTIME} \
      --number-participants ${NUMPART} \
      --message-count ${MSGCOUNT} \
      --message-size ${MSGSIZE} \
      --write-csv ${CSVRUN2}

python diff.py ${CSVRUN1} ${CSVRUN2} ${CSVDIFF}
