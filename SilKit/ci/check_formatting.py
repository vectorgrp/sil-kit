#!/ usr / bin / env python3

# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
fileExtensions = [".cpp", ".ipp", ".c", ".hpp", ".h"]
dirs = ["SilKit", "Demos", "Utilities"]


def get_files_all(args):
    print("Getting all files")
    files = []
    for directory in dirs:
        rootPath = Path("./" + directory)
        for ext in fileExtensions:
            files = files + (sorted(rootPath.rglob('*' + ext)))

    return files


def get_files_commit(args):
    files = []
    print(f"Getting files for commit {args.commit}")

    git_difftree = subprocess.run(['git', 'diff-tree', '--no-commit-id', '--name-only', args.commit ,'-r'],
                                  capture_output=True, encoding='utf-8')

    if git_difftree.returncode != 0:
        die(64, f"Git difftree had an error:\n{git_difftree.stderr}")
    for file in git_difftree.stdout.splitlines():
        files.append(Path(file))
    return files


def get_files_pr(args):

    url = 'https://api.github.com/repos/' + args.repo + '/pulls/' + args.pr + '/files'

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
        
        files.append(Path(fileObject["filename"]))

    if max_page > 1:
        print(f"Need to check {max_page} pages")
        for page in range(2, max_page+1):
            page_url = url + f'?page={page}'
            r = requests.get(page_url)
            for fileObject in r.json():
                files.append(Path(fileObject["filename"]))

    print(f"Found {len(files)} files")
    return files

def get_files_changes(args):
    print("Format all changed files!")
    files = []

    # get unstaged + modified files
    git_lsfiles = subprocess.run(['git', 'ls-files', '--modified'],
                                  capture_output=True, encoding='utf-8')
    if git_lsfiles.returncode != 0:
        die(64, f"Git difftree had an error:\n{git_lsfiles.stderr}")
    for file in git_lsfiles.stdout.splitlines():
        print(file)
        files.append(Path(file))

    # Get staged + modified files
    git_diff = subprocess.run(['git', 'diff', '--cached', '--name-only'],
                                  capture_output=True, encoding='utf-8')
    if git_diff.returncode != 0:
        die(64, f"Git diff had an error:\n{git_diff.stderr}")
    for file in git_diff.stdout.splitlines():
        print(file)
        files.append(Path(file))

    return files

def check_clang_format_version():
    # Check for supported clang - format version
    format_version = subprocess.run([CLANG_FORMAT, '--version'], capture_output=True, encoding='utf-8')

    version_reg = re.compile('^.* clang-format version (\d+)\.(\d+)\.(\d+).*')
    version = re.match(version_reg, format_version.stdout)

    if version is None or len(version.groups()) != 3:
        die(2, "ERROR: Could not get the clang-format version!")

    major, minor, patch = version.group(1,2,3)
    if int(major) < 13:
        die(3, "clang{} not supported!\r\n       Minimum supported version is clang-{}!", major, CLANG_VERSION)

    info("clang-format-{}.{}.{} found!", major, minor, patch)


def format_files(files: list, dryrun: bool):
    formattingCorrect = True
    totalFiles = 0
    totalWarnings = 0

    clang_format_cmd = [CLANG_FORMAT]
    if dryrun:
        clang_format_cmd.append('--Werror')
        clang_format_cmd.append('--dry-run')
    clang_format_cmd = clang_format_cmd + ['-i', '--style=file']
    print(clang_format_cmd)

    for file in files:

        if any(p in file.parts for p in dirs) and file.suffix in fileExtensions:

            totalFiles = totalFiles + 1
            formatResult = subprocess.run(clang_format_cmd + [str(file)], capture_output=True, encoding='utf-8')
            if formatResult.returncode != 0 and dryrun == True:
                totalWarnings = totalWarnings + 1
                warn("File not formatted correctly: {}", file)
        else:
            print(f"Skip: {file}")

    if dryrun:
        info("{} files checked, {} produced a warning!", totalFiles, totalWarnings)
    else:
        info("{} files formatted, please check the result via git diff", totalFiles)

    return formattingCorrect

def main():
    if which(CLANG_FORMAT) is None:
        warn("No {} found!", CLANG_FORMAT)
        die(1, "Please install {}!", CLANG_FORMAT)

    parser = argparse.ArgumentParser(
            prog="ClangFormatRunner",
            description="Run clang tidy on select source files")
    subparsers = parser.add_subparsers()

    parser.add_argument(
        '--dryrun', help="Only produce warnings", action='store_true')
    parser.set_defaults(func=get_files_all)

    parser_pr = subparsers.add_parser('pr', help='Reformat a Github PR')
    parser_pr.add_argument(
        'pr', help="The Github PR to be reformatted", type=str)
    parser_pr.add_argument(
        'repo', help="The Github repo to be reformatted", type=str)
    parser_pr.set_defaults(func=get_files_pr)

    parser_commit = subparsers.add_parser('commit', help='Reformat a Git commit to ammend/fixup')
    parser_commit.add_argument(
            'commit', help="The Git commit to be reformatted")
    parser_commit.set_defaults(func=get_files_commit)

    parser_changes = subparsers.add_parser('changes', help='Reformat a Git commit to ammend/fixup')
    parser_changes.set_defaults(func=get_files_changes)

    args = parser.parse_args()

    check_clang_format_version()
    ############################################################################################################################

    files = args.func(args)

    formattingCorrect = format_files(files, args.dryrun)
    if formattingCorrect is False:
        # Only warn for now
        warn("Formatting for one or more SilKit source code files not correct.!")
        warn("Please format your source code properly using the SilKit .clang-format config file!")
        ret_code = 64 if args.dryrun else 0
        exit(ret_code)

    info("All source code files properly formatted!")
    exit(0)

if __name__=="__main__":
    main()
