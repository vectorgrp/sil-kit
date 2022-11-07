#!/bin/sh
#Usage: ./run-bench-msg-size-scaling.sh <path/to/SilKitDemoBenchmark> <path/to/result.csv> [<path/to/SilKitConfig>]

EXE=$1
CSVFILE=$2
CONFIGFILE=${3:-}
CONFIGARG=""
if [ ! -z "${CONFIGFILE}" ]; then
    CONFIGARG="--configuration ${CONFIGFILE}"
fi

REPEAT=50
SIMTIME=5
NUMPART=2
MSGCOUNT=1

for msgsize in 1 10 100 1000 2000 5000 10000 20000 50000 100000
do
  echo "Run SilKitBenchmarkDemo with RUNS=${REPEAT}, T=${SIMTIME}s, PARTICIPANTS=${NUMPART}, MSGCOUNT=${MSGCOUNT}, MSGSIZE=${msgsize}B, CONFIG=${CONFIGFILE}, CSV=${CSVFILE}"
  $EXE --number-simulation-runs ${REPEAT} \
       --simulation-duration ${SIMTIME} \
       --number-participants ${NUMPART} \
       --message-count ${MSGCOUNT} \
       --message-size ${msgsize} \
       --write-csv ${CSVFILE} \
       ${CONFIGARG} > /dev/null
       
done
