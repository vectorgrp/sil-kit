# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

from git import Repo
import os
import sys
import subprocess
import threading
import signal
import csv
import argparse
import platform

##### function definitions #####

def get_command(command, args, bin_dir):
    return os.path.join(bin_dir, command) + " " + " ".join(args)

def cloneAndBuild(silkit_dir: str, refVersion: str, build_dir: str, useRefCommit=False):
    # clone from GitHub
    repo_url = "https://github.com/vectorgrp/sil-kit.git"
    repo = Repo.clone_from(repo_url, silkit_dir)
        
    if useRefCommit:
        print("Checking out tag/commit id " + refVersion + ".")        
        repo.git.checkout(refVersion)  

    # initialize submodules
    output = repo.git.submodule('update', '--init', '--recursive')
    print(output)

    # build    
    os.makedirs(build_dir)
    subprocess.run(['cmake', "-DCMAKE_BUILD_TYPE=Release", "-DSILKIT_BUILD_TESTS=OFF"  , ".."], cwd=build_dir, check=True)
    subprocess.run(['cmake', "--build", ".", "--config Release"], cwd=build_dir, check=True)

def startRegistry(bin_dir: str, verbose: bool):
    cmd = get_command("sil-kit-registry", ["--log", "off"], bin_dir)
    process = subprocess.Popen(
        cmd, 
        stdout=None if verbose else subprocess.DEVNULL,
    )
    return process.pid

def killProcess(pid: int):
    os.kill(pid, signal.SIGTERM)

def testLatency(bin_dir: str, pathToDir: str, verbose: bool):
    resFile = os.path.join(pathToDir, latencyFile)
    if os.path.isfile(resFile):
        os.remove(resFile)

    # TODO: Use original parameter set
    #optionsLatency = ["--message-size", "10"] + ["--message-count", "1000000"]
    optionsLatency = ["--message-size", "10"] + ["--message-count", "100"] 
    receiver = subprocess.Popen(
        args=get_command("SilKitDemoLatency", ["--isReceiver"] + optionsLatency, bin_dir), 
        stdout=None if verbose else subprocess.DEVNULL,
    )

    sender = subprocess.Popen(
        args=get_command("SilKitDemoLatency", ["--write-csv", resFile] + optionsLatency, bin_dir), 
        stdout=None if verbose else subprocess.DEVNULL,
    )
    receiver.communicate()
    sender.communicate()

def testThroughputLargeMsg(bin_dir: str, pathToDir: str, verbose: bool):
    resFile = os.path.join(pathToDir, throughputLargeMsgFile)
    if os.path.isfile(resFile):
        os.remove(resFile)

    # TODO: Use original parameter set
    #optionsThroughputLargeMsg = ["--message-size", "100000"] + ["--message-count", "1"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", resFile]
    optionsThroughputLargeMsg = ["--message-size", "100000"] + ["--message-count", "1"] + ["--simulation-duration","1"] + ["--number-simulation-runs", "5"] + ["--write-csv", resFile]
    benchmark = subprocess.Popen(
        args=get_command("SilKitDemoBenchmark", optionsThroughputLargeMsg, bin_dir), 
        stdout=None if verbose else subprocess.DEVNULL,
    )
    benchmark.communicate()

def testThroughputSmallMsg(bin_dir: str, pathToDir: str, verbose: bool):
    resFile = os.path.join(pathToDir, throughputSmallMsgFile)
    if os.path.isfile(resFile):
        os.remove(resFile)

    # TODO: Use original parameter set
    #optionsThroughputSmallMsg = ["--message-size", "10"] + ["--message-count", "10"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", resFile]
    optionsThroughputSmallMsg = ["--message-size", "10"] + ["--message-count", "10"] + ["--simulation-duration","1"] + ["--number-simulation-runs", "5"] + ["--write-csv", resFile]
    benchmark = subprocess.Popen(
        args=get_command("SilKitDemoBenchmark", optionsThroughputSmallMsg, bin_dir), 
        stdout=None if verbose else subprocess.DEVNULL,
    )
    benchmark.communicate()

def runTests(bin_dir: str, pathToDir: str, verbose: bool):
    sil_kit_registry_pid = startRegistry(bin_dir, verbose)
    testLatency(bin_dir, pathToDir, verbose)
    testThroughputLargeMsg(bin_dir, pathToDir, verbose)
    testThroughputSmallMsg(bin_dir, pathToDir, verbose)
    killProcess(sil_kit_registry_pid)

def readKpi(path: str, kpiLabel: str):
    with open(path) as csv_file:
        lines = csv_file.readlines()[1:] #skip comment
        data = csv.DictReader(lines, delimiter=';', skipinitialspace=True)    
        vals = [float(row[kpiLabel]) for row in data]
        kpiValue = vals[0]          
    return kpiValue

def assessKpis(ref_kpi_dir: str, kpi_dir: str):
    # get reference kpi values
    latencyRefMean = readKpi(os.path.join(ref_kpi_dir, latencyFile), "latency(us)")
    latencyRefErr = readKpi(os.path.join(ref_kpi_dir, latencyFile), "latency_err")

    throughputLargeRefMean = readKpi(os.path.join(ref_kpi_dir, throughputLargeMsgFile), "throughput(MiB/s)")
    throughputLargeRefErr = readKpi(os.path.join(ref_kpi_dir, throughputLargeMsgFile), "throughput_err")

    throughputSmallRefMean = readKpi(os.path.join(ref_kpi_dir, throughputSmallMsgFile), "throughput(MiB/s)")
    throughputSmallRefErr = readKpi(os.path.join(ref_kpi_dir, throughputSmallMsgFile), "throughput_err")

    # compute thresholds (2 sigma rule)
    sigma = 2.0
    latencyRef = latencyRefMean + sigma * latencyRefErr
    throughputLargeMsgRef = throughputLargeRefMean - sigma * throughputLargeRefErr
    throughputSmallMsgRef = throughputSmallRefMean - sigma * throughputSmallRefErr

    # get kpi values
    latency = readKpi(os.path.join(kpi_dir, latencyFile), "latency(us)")
    throughputLargeMsg = readKpi(os.path.join(kpi_dir, throughputLargeMsgFile), "throughput(MiB/s)")
    throughputSmallMsg = readKpi(os.path.join(kpi_dir, throughputSmallMsgFile), "throughput(MiB/s)")

    # assess and report
    testStr = ""

    print("\n" + "----- Test Report (start) -----" + "\n")

    if latency < latencyRef:
        testStr = "PASSED"
    else:
        testStr = "FAILED"
    print("Latency:                     " + testStr + " with " + str(latency) + " us    (reference value: " + str(latencyRef) + " us).")

    if throughputLargeMsg > throughputLargeMsgRef:
        testStr = "PASSED"
    else:
        testStr = "FAILED"
    print("Throughput (large messages): " + testStr + " with " + str(throughputLargeMsg) + " MiB/s (reference value: " + str(throughputLargeMsgRef) + " MiB/s).")

    if throughputSmallMsg > throughputSmallMsgRef:
        testStr = "PASSED"
    else:
        testStr = "FAILED"
    print("Throughput (small messages): " + testStr + " with " + str(throughputSmallMsg) + " MiB/s (reference value: " + str(throughputSmallMsgRef) + " MiB/s).")

    print("\n" + "----- Test Report (end) -------")

##### start script #####

latencyFile = "latency.csv"
throughputLargeMsgFile = "throughputLargeMsg.csv"
throughputSmallMsgFile = "throughputSmallMsg.csv"

def main():
    parser = argparse.ArgumentParser(description="Process a reference tag or commit id.")
    parser.add_argument('refVersion', nargs='?', default='v4.0.52', help='Reference tag or commit id')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print output of SIL Kit applications to stdout')
    args = parser.parse_args()

    if len(sys.argv) > 2:
        sys.exit("Only one command line argument (reference tag/commit id) allowed.")

    refVersion = args.refVersion

    # set reference kpis if not available yet
    ref_kpi_dir = os.path.join(os.getcwd(), "kpis_ref")
    if not os.path.isdir(ref_kpi_dir):
        os.makedirs(ref_kpi_dir)

        silkit_dir = os.path.join(os.getcwd(), "sil-kit-reference")
        build_dir = os.path.join(silkit_dir, "build")
        bin_dir = os.path.join(build_dir, "Release")    
        
        if not os.path.isdir(silkit_dir):
            cloneAndBuild(silkit_dir, refVersion, build_dir, True)
        else:
            print("SIL Kit (reference version) has already been cloned and built.")

        runTests(bin_dir, ref_kpi_dir, args.verbose)
    else:
        # TODO If something went wrong in creating the ref. KPIs (e.g. registry collision, build failure,...), the folder exists but the result files not
        print("Reference KPIs already existent.")

    # get kpis of current SIL Kit version
    kpi_dir = os.path.join(os.getcwd(), "kpis")
    if not os.path.isdir(kpi_dir):
        os.makedirs(kpi_dir)

    silkit_dir = os.path.join(os.getcwd(), "sil-kit")
    build_dir = os.path.join(silkit_dir, "build")
    bin_dir = os.path.join(build_dir, "Release")

    if not os.path.isdir(silkit_dir):
        cloneAndBuild(silkit_dir, refVersion, build_dir, False)
    else:
        print("SIL Kit (current version) has already been cloned and built.")

    runTests(bin_dir, kpi_dir, args.verbose)

    assessKpis(ref_kpi_dir, kpi_dir)

if __name__ == "__main__":
    main()