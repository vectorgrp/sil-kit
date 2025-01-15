<!--
SPDX-FileCopyrightText: 2023 Vector Informatik GmbH

SPDX-License-Identifier: MIT
-->

Message size scaling helper scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Collect timings for different message sizes and generate a plot with the results.
Run in a MinGW console with ``./benchmark.sh <path/to/SilKitDemoBenchmark.exe>``.

- ``run-bench-msg-size-scaling.sh``:
  Usage: ``./run-bench-msg-size-scaling.sh <path/to/SilKitDemoBenchmark> <path/to/result.csv> [<path/to/SilKitConfig>]``
  Starts a given SilKitDemoBenchmark executable with messages sizes from 1B to 100kB and saves the timings in a
  given csv file. Optionally accepts a SIL Kit configuration file

- ``benchmark.sh``:
  Needs the path to the SilKitDemoBenchmark as input argument. Uses ``run-bench-msg-size-scaling.sh`` to run batches 
  of SilKitDemoBenchmark w/wo DomainSockets and creates a result plot.
  
- ``plot-msg-size-scaling.gp``:
  This gnupot script  plots the results for throughput, message rate, and speedup to ``result-msg-size-scaling.pdf``.
