#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import requests
import argparse
import os

isCI = os.getenv('CI')
INFO_PREFIX = "::notice ::" if isCI != None else "INFO: "
WARN_PREFIX = "::warning ::" if isCI != None else "WARNING: "
ERROR_PREFIX = "::error ::" if isCI != None else "ERROR: "

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

# File Set
exceptional_files = {'README.rst', 'CHANGELOG.rst', 'LICENSE', 'CONTRIBUTING.md',
                     'SilKitVersion.cmake'}

parser = argparse.ArgumentParser(prog="JobStatusChecker",
                                 description="Check Job Status of a specific github workflow run")
parser.add_argument('repo', type=str)
parser.add_argument('PR', type=str)
args = parser.parse_args()

run_builds = "false"

url = 'https://api.github.com/repos/' + args.repo + '/pulls/' + args.PR + '/files'

log("Checking at {}".format(url))

r = requests.get(url, verify=False)

for fileObject in r.json():

    file_path = fileObject["filename"];
    file = file_path.split(sep="/")[-1]

    if file not in exceptional_files:
        run_builds = "true"
        break

log("Builds should run: {}".format(run_builds))

if isCI != None:
    log("Setting GITHUB_OUTPUT!")
    with open(os.environ["GITHUB_OUTPUT"], 'a') as f:
        print("run_builds={}".format(run_builds), file=f)
