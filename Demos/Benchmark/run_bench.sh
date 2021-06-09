#!/bin/sh
#usage: run_bench.sh path/to/IbBenchmark.exe
set -e
set -u
EXE=$1
REPEAT=3
SIMTIME=1
export IB_EXTENSION_PATH=$(dirname $EXE) #for vib-registry.so
for msgsize in 1 64 1024
do
	for msg in 1 10 100
	do
		for part in  2 4 8 16
		do
			$EXE  VAsio ${REPEAT} ${SIMTIME} ${part} ${msg} ${msgsize} | tee bench_msg${msg}_msgsize${msgsize}_part${part}.txt
		done
	done
done
