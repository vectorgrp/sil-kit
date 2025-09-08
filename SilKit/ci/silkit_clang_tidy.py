#! /bin/env python3

# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import argparse
import json
import os
import subprocess
import sys

from multiprocessing import Pool, cpu_count
from pathlib import Path

isCI = os.getenv('CI')
INFO_PREFIX = "::notice ::" if isCI is not None else "INFO: "
WARN_PREFIX = "::warning ::" if isCI is not None else "WARNING: "
ERROR_PREFIX = "::error ::" if isCI is not None else "ERROR: "


# Convenience
def log(fmt, *args):
    print(fmt.format(*args))


def info(fmt, *args):
    log(INFO_PREFIX + fmt, *args)


def warn(fmt, *args):
    log(WARN_PREFIX + fmt, *args)


def die(status, fmt, *args):
    log(ERROR_PREFIX + fmt, *args)
    sys.exit(status)


def run_clang_tidy(source_file: Path, outdir: Path):

    source_file_str = str(outdir / f"{source_file.name}_fixes.yaml")
    cmd_args = ['clang-tidy', source_file, '--header-filter', '\'^.*SilKit.*\'',
                '-p', 'build_tidy/',
                '--export-fixes', source_file_str,
                '--quiet']

    clang_tidy = subprocess.run(cmd_args,
                                capture_output=True, encoding='utf-8')

    if clang_tidy.stdout != "" and "ThirdParty" not in clang_tidy.stdout:
        warn(f"{source_file}:\n{clang_tidy.stdout}", flush=True)


def main():

    parser = argparse.ArgumentParser(
            prog="ClangTidyRunner",
            description="Run clang tidy on select source files")

    parser.add_argument(
        'builddir', help="The build dir containing the compile_commands.json", type=Path)
    parser.add_argument(
        'outdir', help="The directory to store reports in", type=Path)

    args = parser.parse_args()
    with open(args.builddir / 'compile_commands.json', 'r') as f:
        cc = json.load(f)

    source_files = []
    for file_entry in cc:
        # Exclude ThirdParty Stuff
        filename = file_entry["file"]
        if "ThirdParty" not in filename:

            # Skip F|I|UTests due to gmock macro hell
            if "Test_" in filename:
                continue
            source_files.append(filename)

    print(f"Processing {len(source_files)} cpp files!")

    global parallel_tidy

    def parallel_tidy(source_file):
        run_clang_tidy(Path(source_file), args.outdir)

    num_cpus = cpu_count()

    with Pool(num_cpus) as p:
        p.map(parallel_tidy, source_files)


if __name__ == "__main__":
    main()
