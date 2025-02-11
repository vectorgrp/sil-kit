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

##### function definitions #####

def cloneAndBuild(useRefCommit=False):
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
    subprocess.run(['cmake', "-DCMAKE_BUILD_TYPE=Release", ".."], cwd=build_dir, check=True)
    subprocess.run(['cmake', "--build", "."], cwd=build_dir, check=True)

def startRegistry():
    global sil_kit_registry_pid
    process = subprocess.Popen(["./sil-kit-registry","--log","off"], cwd=app_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    sil_kit_registry_pid = process.pid
    process.communicate()

def startRegistryThread():
    sil_kit_registry_pid = None
    registry = threading.Thread(target=startRegistry)
    registry.start()
    return sil_kit_registry_pid

def stopRegistryThread(pid: int):
    os.kill(pid, signal.SIGTERM)

def testLatency(pathToDir: str):
    resFile = os.path.join(pathToDir, latencyFile)

    if os.path.isfile(resFile):
        os.remove(resFile)

    optionsLatency = ["--message-size", "10"] + ["--message-count", "1000000"]
    def start_receiver():
        subprocess.run(["./SilKitDemoLatency", "--isReceiver"] + optionsLatency, cwd=app_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    
    def start_sender():
        subprocess.run(["./SilKitDemoLatency", "--write-csv", resFile] + optionsLatency, cwd=app_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    receiver = threading.Thread(target=start_receiver)
    sender = threading.Thread(target=start_sender)
    receiver.start()
    sender.start()

    sender.join()
    receiver.join()

def testThroughputLargeMsg(pathToDir: str):
    resFile = os.path.join(pathToDir, throughputLargeMsgFile)

    if os.path.isfile(resFile):
        os.remove(resFile)

    optionsThroughputLargeMsg = ["--message-size", "100000"] + ["--message-count", "1"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", resFile]
    subprocess.run(["./SilKitDemoBenchmark"] + optionsThroughputLargeMsg, cwd=app_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

def testThroughputSmallMsg(pathToDir: str):
    resFile = os.path.join(pathToDir, throughputSmallMsgFile)

    if os.path.isfile(resFile):
        os.remove(resFile)

    optionsThroughputSmallMsg = ["--message-size", "10"] + ["--message-count", "10"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", resFile]
    subprocess.run(["./SilKitDemoBenchmark"] + optionsThroughputSmallMsg, cwd=app_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

def runTests(pathToDir: str):
    testLatency(pathToDir)
    testThroughputLargeMsg(pathToDir)
    testThroughputSmallMsg(pathToDir)

def readKpi(path: str, kpiLabel: str):
    with open(path) as csv_file:
        lines = csv_file.readlines()[1:] #skip comment
        data = csv.DictReader(lines, delimiter=';', skipinitialspace=True)    
        vals = [float(row[kpiLabel]) for row in data]
        kpiValue = vals[0]          
    return kpiValue

def assessKpis():
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

refVersion = "v4.0.52"
if len(sys.argv) == 2:
    refVersion = sys.argv[-1]
elif len(sys.argv) > 2:
    sys.exit("Only one command line argument (reference tag/commit id) allowed.")

# set reference kpis if not available yet
ref_kpi_dir = os.path.join(os.getcwd(), "kpis_ref")
if not os.path.isdir(ref_kpi_dir):
    os.makedirs(ref_kpi_dir)

    silkit_dir = os.path.join(os.getcwd(), "sil-kit-reference")
    build_dir = os.path.join(silkit_dir, "build")
    app_dir = os.path.join(build_dir, "Release")    
    
    if not os.path.isdir(silkit_dir):
        cloneAndBuild(True)
    else:
        print("SIL Kit (reference version) has already been cloned and built.")

    sil_kit_registry_pid = startRegistryThread()
    runTests(ref_kpi_dir)
    stopRegistryThread(sil_kit_registry_pid)
else:
    print("Reference KPIs already existent.")

# get kpis of current SIL Kit version
kpi_dir = os.path.join(os.getcwd(), "kpis")
if not os.path.isdir(kpi_dir):
    os.makedirs(kpi_dir)

silkit_dir = os.path.join(os.getcwd(), "sil-kit")
build_dir = os.path.join(silkit_dir, "build")
app_dir = os.path.join(build_dir, "Release")

if not os.path.isdir(silkit_dir):
    cloneAndBuild()
else:
    print("SIL Kit (current version) has already been cloned and built.")

sil_kit_registry_pid = startRegistryThread()
runTests(kpi_dir)
stopRegistryThread(sil_kit_registry_pid)

# assess kpis
assessKpis()