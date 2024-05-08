#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT
from pathlib import Path
from shutil import which
import re
import subprocess
import sys

## Convenience
def log(fmt, *args):
    print(fmt.format(*args))

def info(fmt, *args):
    log("INFO: " + fmt, *args)

def warn(fmt, *args):
    log("::warning:: " + fmt, *args)

def die(status, fmt, *args):
    log("ERROR: " + fmt, *args)
    sys.exit(status)

## Check if clang-format is installed

if which('clang-format') is None:
    warn("No clang-format found!")
    die(1, "Please install clang-format!")

# Check for supported clang-format version
format_version = subprocess.run(['clang-format', '--version'], capture_output=True)

version_reg = re.compile('^.* clang-format version (\d+)\.(\d+)\.(\d+).*')
version = re.match(version_reg, format_version.stdout.decode('utf-8'))

if version is None or len(version.groups()) != 3:
    die(2, "ERROR: Could not get the clang-format version!")

major, minor, patch = version.group(1,2,3)
if int(major) < 13:
    die(3, "clang{} not supported!\r\n       Minimum supported version is clang14!", major)

info("clang-format-{}.{}.{} found!", major, minor, patch)

############################################################################################################################
fileExtensions = ["*.cpp", "*.ipp", "*.c", "*.hpp", "*.h"]
dirs = ["SilKit", "Demos", "Utilities"]

formatting_correct = True

# Open LogFile to get some data on our poorly formatted files!
with open('FormatLog.txt', 'w') as fLog:

# Check  the Formatting!
    for directory in dirs:
        rootPath = Path("./" + directory)
        for ext in fileExtensions:
            files = sorted(rootPath.rglob(ext))
            #print("INFO: Found {} {} files in {}!".format(len(files), ext, directory))

            for file in files:
                format_result = subprocess.run(['clang-format', '--Werror', '--dry-run', '-i', '--style=file', file], capture_output=True)
                if format_result.returncode != 0:
                    formatting_correct = False
                    fLog.write("{}\n".format(file));

if formatting_correct is False:
    warn("Formatting for one or more SilKit source code files not correct. Please consult the artifact \"FormatLog.txt\" to get the full list of files!")
    die(4, "Please format your source code properly using the SilKit .clang-format config file!")

info("All source code files properly formatted!")
exit(0)
