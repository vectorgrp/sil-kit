#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import requests
import argparse
import os
import sys

isCI = os.getenv('CI')
INFO_PREFIX = "::notice ::" if isCI != None else "INFO: "
WARN_PREFIX = "::warning ::" if isCI != None else "WARNING: "
ERROR_PREFIX = "::error ::" if isCI != None else "ERROR: "


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


def check_run_build(url: str):
    run_builds = "false"
    files_url = url + '/files'
    r = requests.get(files_url, verify=False)

    for fileObject in r.json():

        file_path = fileObject["filename"]
        file = file_path.split(sep="/")[-1]

        if file not in exceptional_files:
            run_builds = "true"
            break

    log("Builds should run: {}".format(run_builds))

    if isCI is not None:
        log("Setting GITHUB_OUTPUT!")
        with open(os.environ["GITHUB_OUTPUT"], 'a') as f:
            print("run_builds={}".format(run_builds), file=f)


def check_dco(url: str):
    commits_url = url + '/commits'

    r = requests.get(commits_url, verify=False)

    all_commits_signed = True
    for commitObject in r.json():
        committer = commitObject["commit"]["committer"]
        author = commitObject["commit"]["author"]
        message = commitObject["commit"]["message"]

        committer_error = ""
        author_error = ""

        # Do not check fixup messages
        if "fixup" not in message:

            # Check committer only if it they are different
            if committer["email"] != author["email"] or committer["name"] != author["name"]:
                sign_off = f'Signed-off-by: {committer["name"]} <{committer["email"]}>'
                if sign_off not in message:
                    comitter_error = f'Signed-off-by from committer {committer["name"]} missing! Merging blocked!'

            sign_off = f'Signed-off-by: {author["name"]} <{author["email"]}>'
            if sign_off not in message:
                author_error = f'Signed-off-by from author {author["name"]} missing, Merging Blocked!'

            if committer_error or author_error:
                all_commits_signed = False
                warn(f'{commitObject["sha"][0:7]}: {author_error} {committer_error}')

    if all_commits_signed is False:
        die(66, "Not all commits Signed-off by their respective author/committer")

# File Set
exceptional_files = {'README.rst', 'latest.md', 'LICENSE', 'CONTRIBUTING.md',
                     'SilKitVersion.cmake'}

parser = argparse.ArgumentParser(prog="JobStatusChecker",
                                 description="Check Job Status of a specific github workflow run")
parser.add_argument('repo', type=str)
parser.add_argument('PR', type=str)
args = parser.parse_args()

url = 'https://api.github.com/repos/' + args.repo + '/pulls/' + args.PR

log("Checking at {}".format(url))

check_dco(url)
check_run_build(url)

