# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import os
import sys
import subprocess
import signal
import csv
import argparse

from git import Repo  # PyPI: GitPython


##### function definitions #####

def get_command(command, args, bin_dir):
    return os.path.join(bin_dir, command) + " " + " ".join(args)


def clone_and_build(silkit_dir: str, ref_version: str, build_dir: str, use_ref_commit=False):
    # clone from GitHub
    repo_url = "https://github.com/vectorgrp/sil-kit.git"
    repo = Repo.clone_from(repo_url, silkit_dir)

    if use_ref_commit:
        print("Checking out tag/commit id " + ref_version + ".")
        repo.git.checkout(ref_version)

        # initialize submodules
    output = repo.git.submodule('update', '--init', '--recursive')
    print(output)

    # build    
    os.makedirs(build_dir)
    subprocess.run(['cmake', "-DCMAKE_BUILD_TYPE=Release", "-DSILKIT_BUILD_TESTS=OFF", ".."], cwd=build_dir, check=True)
    subprocess.run(['cmake', "--build", ".", "--config Release"], cwd=build_dir, check=True)


def start_registry(bin_dir: str, verbose: bool):
    cmd = get_command("sil-kit-registry", ["--log", "off"], bin_dir)
    process = subprocess.Popen(
        cmd,
        stdout=None if verbose else subprocess.DEVNULL,
    )
    return process.pid


def kill_process(pid: int):
    os.kill(pid, signal.SIGTERM)


def test_latency(bin_dir: str, path_to_dir: str, verbose: bool):
    res_file = os.path.join(path_to_dir, latency_file)
    if os.path.isfile(res_file):
        os.remove(res_file)

    # TODO: Use original parameter set
    # options_latency = ["--message-size", "10"] + ["--message-count", "1000000"]
    options_latency = ["--message-size", "10"] + ["--message-count", "100"]

    receiver = subprocess.Popen(
        args=get_command("SilKitDemoLatency", ["--isReceiver"] + options_latency, bin_dir),
        stdout=None if verbose else subprocess.DEVNULL,
    )

    sender = subprocess.Popen(
        args=get_command("SilKitDemoLatency", ["--write-csv", res_file] + options_latency, bin_dir),
        stdout=None if verbose else subprocess.DEVNULL,
    )

    receiver.communicate()
    sender.communicate()


def test_throughput_large_msg(bin_dir: str, path_to_dir: str, verbose: bool):
    res_file = os.path.join(path_to_dir, throughput_large_msg_file)
    if os.path.isfile(res_file):
        os.remove(res_file)

    # TODO: Use original parameter set
    # options_throughput_large_msg = ["--message-size", "100000"] + ["--message-count", "1"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", res_file]
    options_throughput_large_msg = ["--message-size", "100000"] + ["--message-count", "1"] + ["--simulation-duration",
                                                                                              "1"] + [
                                       "--number-simulation-runs", "5"] + ["--write-csv", res_file]

    benchmark = subprocess.Popen(
        args=get_command("SilKitDemoBenchmark", options_throughput_large_msg, bin_dir),
        stdout=None if verbose else subprocess.DEVNULL,
    )

    benchmark.communicate()


def test_throughput_small_msg(bin_dir: str, path_to_dir: str, verbose: bool):
    res_file = os.path.join(path_to_dir, throughput_small_msg_file)
    if os.path.isfile(res_file):
        os.remove(res_file)

    # TODO: Use original parameter set
    # options_throughput_small_msg = ["--message-size", "10"] + ["--message-count", "10"] + ["--simulation-duration","10"] + ["--number-simulation-runs", "50"] + ["--write-csv", res_file]
    options_throughput_small_msg = ["--message-size", "10"] + ["--message-count", "10"] + ["--simulation-duration",
                                                                                           "1"] + [
                                       "--number-simulation-runs", "5"] + ["--write-csv", res_file]

    benchmark = subprocess.Popen(
        args=get_command("SilKitDemoBenchmark", options_throughput_small_msg, bin_dir),
        stdout=None if verbose else subprocess.DEVNULL,
    )

    benchmark.communicate()


def run_tests(bin_dir: str, path_to_dir: str, verbose: bool):
    sil_kit_registry_pid = start_registry(bin_dir, verbose)
    test_latency(bin_dir, path_to_dir, verbose)
    test_throughput_large_msg(bin_dir, path_to_dir, verbose)
    test_throughput_small_msg(bin_dir, path_to_dir, verbose)
    kill_process(sil_kit_registry_pid)


def read_kpi(path: str, kpi_label: str):
    with open(path) as csv_file:
        lines = csv_file.readlines()[1:]  # skip comment
        data = csv.DictReader(lines, delimiter=';', skipinitialspace=True)
        vals = [float(row[kpi_label]) for row in data]
        kpi_value = vals[0]

    return kpi_value


def assess_kpis(ref_kpi_dir: str, kpi_dir: str):
    # get reference kpi values
    latency_ref_mean = read_kpi(os.path.join(ref_kpi_dir, latency_file), "latency(us)")
    latency_ref_err = read_kpi(os.path.join(ref_kpi_dir, latency_file), "latency_err")

    throughput_large_ref_mean = read_kpi(os.path.join(ref_kpi_dir, throughput_large_msg_file), "throughput(MiB/s)")
    throughput_large_ref_err = read_kpi(os.path.join(ref_kpi_dir, throughput_large_msg_file), "throughput_err")

    throughput_small_ref_mean = read_kpi(os.path.join(ref_kpi_dir, throughput_small_msg_file), "throughput(MiB/s)")
    throughput_small_ref_err = read_kpi(os.path.join(ref_kpi_dir, throughput_small_msg_file), "throughput_err")

    # compute thresholds (2 sigma rule)
    sigma = 2.0
    latency_ref = latency_ref_mean + sigma * latency_ref_err
    throughput_large_msg_ref = throughput_large_ref_mean - sigma * throughput_large_ref_err
    throughput_small_msg_ref = throughput_small_ref_mean - sigma * throughput_small_ref_err

    # get kpi values
    latency = read_kpi(os.path.join(kpi_dir, latency_file), "latency(us)")
    throughput_large_msg = read_kpi(os.path.join(kpi_dir, throughput_large_msg_file), "throughput(MiB/s)")
    throughput_small_msg = read_kpi(os.path.join(kpi_dir, throughput_small_msg_file), "throughput(MiB/s)")

    # assess and report
    test_str = ""

    print("\n" + "----- Test Report (start) -----" + "\n")

    if latency < latency_ref:
        test_str = "PASSED"
    else:
        test_str = "FAILED"
    print("Latency:                     " + test_str + " with " + str(latency) + " us    (reference value: " + str(
        latency_ref) + " us).")

    if throughput_large_msg > throughput_large_msg_ref:
        test_str = "PASSED"
    else:
        test_str = "FAILED"
    print("Throughput (large messages): " + test_str + " with " + str(
        throughput_large_msg) + " MiB/s (reference value: " + str(throughput_large_msg_ref) + " MiB/s).")

    if throughput_small_msg > throughput_small_msg_ref:
        test_str = "PASSED"
    else:
        test_str = "FAILED"
    print("Throughput (small messages): " + test_str + " with " + str(
        throughput_small_msg) + " MiB/s (reference value: " + str(throughput_small_msg_ref) + " MiB/s).")

    print("\n" + "----- Test Report (end) -------")


##### start script #####

latency_file = "latency.csv"
throughput_large_msg_file = "throughputLargeMsg.csv"
throughput_small_msg_file = "throughputSmallMsg.csv"


def main():
    parser = argparse.ArgumentParser(description="Process a reference tag or commit id.")
    parser.add_argument('ref-version', nargs='?', default='v4.0.52', help='Reference tag or commit id')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print output of SIL Kit applications to stdout')
    args = parser.parse_args()

    if len(sys.argv) > 2:
        sys.exit("Only one command line argument (reference tag/commit id) allowed.")

    ref_version = args.refVersion

    # set reference kpis if not available yet
    ref_kpi_dir = os.path.join(os.getcwd(), "kpis_ref")
    if not os.path.isdir(ref_kpi_dir):
        os.makedirs(ref_kpi_dir)

        silkit_dir = os.path.join(os.getcwd(), "sil-kit-reference")
        build_dir = os.path.join(silkit_dir, "build")
        bin_dir = os.path.join(build_dir, "Release")

        if not os.path.isdir(silkit_dir):
            clone_and_build(silkit_dir, ref_version, build_dir, True)
        else:
            print("SIL Kit (reference version) has already been cloned and built.")

        run_tests(bin_dir, ref_kpi_dir, args.verbose)
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
        clone_and_build(silkit_dir, ref_version, build_dir, False)
    else:
        print("SIL Kit (current version) has already been cloned and built.")

    run_tests(bin_dir, kpi_dir, args.verbose)

    assess_kpis(ref_kpi_dir, kpi_dir)


if __name__ == "__main__":
    main()
