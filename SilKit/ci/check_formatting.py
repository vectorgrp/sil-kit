#!/ usr / bin / env python3

#SPDX - FileCopyrightText : 2024 Vector Informatik GmbH
#
#SPDX - License - Identifier : MIT
from pathlib import Path
from shutil import which

import argparse
import os
import re
import requests
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

def get_pr_files(pr: str, repo: str):

    url = 'https://api.github.com/repos/' + repo + '/pulls/' + pr + '/files'

    log("Checking at {}".format(url))

    r = requests.get(url)

    max_page = 1
    if 'link' in r.headers:
        last_page_link_header = r.headers["link"].split(',')[-1].split(';')[0]
        page_reg = re.compile('.+page=([0-9]+)>')
        page = re.match(page_reg, last_page_link_header).group(1)

        if page.isnumeric():
            max_page = int(page)

    files = []
    for fileObject in r.json():
        
        files.append(fileObject["filename"])

    if max_page > 1:
        print(f"Need to check {max_page} pages")
        for page in range(2, max_page+1):
            page_url = url + f'?page={page}'
            r = requests.get(page_url)
            for fileObject in r.json():
                files.append(fileObject["filename"])

    print(f"Found {len(files)} files")
    return files

def main():
    if which(CLANG_FORMAT) is None:
        warn("No {} found!", CLANG_FORMAT)
        die(1, "Please install {}!", CLANG_FORMAT)

    parser = argparse.ArgumentParser(
            prog="ClangFormatRunner",
            description="Run clang tidy on select source files")

    parser.add_argument(
        '--pr', help="The Github PR to be reformatted", type=str)
    parser.add_argument(
        '--repo', help="The Github repo to be reformatted", type=str)
    parser.add_argument(
        '--dryrun', help="Only produce warnings", action='store_true')
    args = parser.parse_args()

#Check for supported clang - format version
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

#Check the Formatting !
    files = []

    if args.pr is not None and args.repo is not None:
        files = get_pr_files(args.pr, args.repo)
    else:
        for directory in dirs:
            rootPath = Path("./" + directory)
            for ext in fileExtensions:
                files = files + (sorted(rootPath.rglob(ext)))

    clang_format_cmd = [CLANG_FORMAT]
    if args.dryrun:
        clang_format_cmd.append('--Werror')
        clang_format_cmd.append('--dry-run')

    clang_format_cmd = clang_format_cmd + ['-i', '--style=file']
    print(clang_format_cmd)
    for file in files:

        if any(p in file for p in dirs):

            totalFiles = totalFiles + 1
            formatResult = subprocess.run(clang_format_cmd + [file], capture_output=True, encoding='utf-8')
            if formatResult.returncode != 0:
                formattingCorrect = False
                totalWarnings = totalWarnings + 1
                warn("File not formatted correctly: {}", file)
        else:
            print(f"Skip: {file}")

    info("{} files checked, {} produced a warning!", totalFiles, totalWarnings)
    if formattingCorrect is False:
#Only warn for now
        warn("Formatting for one or more SilKit source code files not correct.!")
        warn("Please format your source code properly using the SilKit .clang-format config file!")
        ret_code = 64 if args.dryrun else 0
        exit(ret_code)

    info("All source code files properly formatted!")
    exit(0)

if __name__=="__main__":
    main()
