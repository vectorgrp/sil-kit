<!--
SPDX-FileCopyrightText: 2023 Vector Informatik GmbH

SPDX-License-Identifier: MIT
-->

Message size scaling helper scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compares timings for two different versions of the SilKitDemoBenchmark and calculates the performance difference.
Run in a MinGW console with 
``./performance-diff.sh <name-version-1> <path/to/SilKitDemoBenchmark1> <name-version-2> <path/to/SilKitDemoBenchmark2> <path/to/diff-result.csv>
The arguments <name-version-1/2> are for naming the csv outputs of the SilKitDemoBenchmark1/2. 

- ``performance-diff.sh``:
  Starts the given two SilKitDemoBenchmark executables and saves the timings in seperate csv files.
  
- ``diff.py``:
  Reads the csv results of SilKitDemoBenchmark1/2 and calculates the performance difference.
  The result is printed and saved to <path/to/result-diff.csv>.
