#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT
from pathlib import Path
from shutil import which
import os
import re
import subprocess
import sys

INFO_PREFIX = "::notice ::" if os.getenv('CI') != None else "INFO: "
WARN_PREFIX = "::warning ::" if os.getenv('CI') != None else "WARNING: "
ERROR_PREFIX = "::error ::" if os.getenv('CI') != None else "ERROR: "

## Convenience
def log(fmt, *args):
    print(fmt.format(*args))

def info(fmt, *args):
    log(INFO_PREFIX + fmt, *args)

def warn(fmt, *args):
    log(WARN_PREFIX + fmt, *args)

def die(status, fmt, *args):
    log(ERROR_PREFIX + fmt, *args)
    sys.exit(status)

## Check if clang-format is installed
CLANG_VERSION = "18"
CLANG_FORMAT = "clang-format-" + CLANG_VERSION

def main():
    if which(CLANG_FORMAT) is None:
        warn("No {} found!", CLANG_FORMAT)
        die(1, "Please install {}!", CLANG_FORMAT)

    # Check for supported clang-format version
    format_version = subprocess.run([CLANG_FORMAT, '--version'], capture_output=True, encoding='utf-8')

    version_reg = re.compile('^.* clang-format version (\d+)\.(\d+)\.(\d+).*')
    version = re.match(version_reg, format_version.stdout)

    if version is None or len(version.groups()) != 3:
        die(2, "ERROR: Could not get the clang-format version!")

    major, minor, patch = version.group(1,2,3)
    if int(major) < 13:
        die(3, "clang{} not supported!\r\n       Minimum supported version is clang-{}!", major, CLANG_VERSION)

    info("clang-format-{}.{}.{} found!", major, minor, patch)

    ############################################################################################################################
    fileExtensions = ["*.cpp", "*.ipp", "*.c", "*.hpp", "*.h"]
    dirs = ["SilKit", "Demos", "Utilities"]

    formattingCorrect = True
    totalFiles = 0
    totalWarnings = 0

    # Check  the Formatting!
    for directory in dirs:
        rootPath = Path("./" + directory)
        for ext in fileExtensions:
            files = sorted(rootPath.rglob(ext))
            #print("INFO: Found {} {} files in {}!".format(len(files), ext, directory))

            for file in files:
                totalFiles = totalFiles + 1
                formatResult = subprocess.run([CLANG_FORMAT, '--Werror', '--dry-run', '-i', '--style=file', file], capture_output=True, encoding='utf-8')
                if formatResult.returncode != 0:
                    formattingCorrect = False
                    totalWarnings = totalWarnings + 1
                    warn("File not formatted correctly: {}", file)

    info("{} files checked, {} produced a warning!", totalFiles, totalWarnings)
    if formattingCorrect is False:
        # Only warn for now
        warn("Formatting for one or more SilKit source code files not correct.!")
        warn("Please format your source code properly using the SilKit .clang-format config file!")
        exit(0)

    info("All source code files properly formatted!")
    exit(0)

if __name__=="__main__":
    main()
