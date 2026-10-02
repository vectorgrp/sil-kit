#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import requests
import argparse
import os

from ci_utils import log, isCI


def check_run_build(url: str):
    run_builds = "false"
    files_url = url + '/files'
    r = requests.get(files_url, verify=False)
    
    try:
        files_data = r.json()
        # Ensure we have a list/array
        if not isinstance(files_data, list):
            log(f"Unexpected response format: {type(files_data)}")
            log(f"Response content: {files_data}")
            return
        
        for fileObject in files_data:
            # Ensure fileObject is a dict
            if not isinstance(fileObject, dict):
                continue
                
            file_path = fileObject.get("filename", "")
            file = file_path.split(sep="/")[-1]

            if file not in exceptional_files:
                run_builds = "true"
                break
    except (ValueError, KeyError) as e:
        log(f"Error processing files response: {e}")
        return

    log("Builds should run: {}".format(run_builds))

    if isCI is not None:
        log("Setting GITHUB_OUTPUT!")
        with open(os.environ["GITHUB_OUTPUT"], 'a') as f:
            print("run_builds={}".format(run_builds), file=f)

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

check_run_build(url)

