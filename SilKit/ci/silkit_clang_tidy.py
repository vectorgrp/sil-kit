#! /bin/env python3
import json
import os
import subprocess
import sys

from multiprocessing import Pool, cpu_count

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


def run_clang_tidy(source_file: str):
    clang_tidy = subprocess.run(['clang-tidy', source_file, '--header-filter', '\'^.*SilKit.*\'',
                                 '-p', 'build_tidy/',
                                 '--quiet'],
                                capture_output=True, encoding='utf-8')

    if clang_tidy.stdout != "":
        print(f"Processing source file {source_file}", flush=True)
        print(f"{source_file} o:\n{clang_tidy.stdout}", flush=True)


with open('build_tidy/compile_commands.json', 'r') as f:
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

num_cpus = cpu_count()
with Pool(num_cpus) as p:
    p.map(run_clang_tidy, source_files)
