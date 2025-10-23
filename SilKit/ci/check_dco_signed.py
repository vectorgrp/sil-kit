#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import requests
import argparse

from ci_utils import die, warn


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
                if sign_off not in message and committer["name"] != "GitHub":
                    comitter_error = f'Signed-off-by from committer {committer["name"]} missing! Merging blocked!'

            sign_off = f'Signed-off-by: {author["name"]} <{author["email"]}>'
            if sign_off not in message:
                author_error = f'Signed-off-by from author {author["name"]} missing, Merging Blocked!'

            if committer_error or author_error:
                all_commits_signed = False
                warn(f'{commitObject["sha"][0:7]}: {author_error} {committer_error}')

    if all_commits_signed is False:
        die(66, "Not all commits Signed-off by their respective author/committer")


def main():

    parser = argparse.ArgumentParser(prog="DCO checker",
                                     description="Check whether the DCO was signed for a PR")
    parser.add_argument('repo', type=str)
    parser.add_argument('PR', type=str)
    args = parser.parse_args()
    url = 'https://api.github.com/repos/' + args.repo + '/pulls/' + args.PR
    check_dco(url)


if __name__ == "__main__":
    main()
