#!/bin/sh

# SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

echo 'Cleanup'
rm ./result-msg-size-scaling.csv
rm ./result-msg-size-scaling-tcp.csv
rm ./result-msg-size-scaling.pdf

EXE=$1

echo 'Run benchmarks with domain sockets ...'
./run-bench-msg-size-scaling.sh $1 ./result-msg-size-scaling.csv 

echo 'Run benchmarks with TCP ...'
./run-bench-msg-size-scaling.sh $1 ./result-msg-size-scaling-tcp.csv ./SilKitConfig_DemoBenchmark_DomainSockets_Off.yaml

echo 'Create plots...'
gnuplot plot-msg-size-scaling.gp
