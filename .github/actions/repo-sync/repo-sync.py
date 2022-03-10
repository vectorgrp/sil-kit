#!/usr/bin/env python3
# Copyright (c) Vector Informatik GmbH. All rights reserved.

import github3
import sys
from urllib.parse import urlparse
import logging
import argparse
import os
from  subprocess import check_output

# setup logging to (stderr)
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', datefmt='%F %H:%M:%S', level=logging.INFO)

info = logging.info
debug = logging.debug
error = logging.error

# globals
REPO_SYNC_APP_ID=os.getenv("REPO_SYNC_APP_ID")
REPO_SYNC_APP_KEY=os.getenv("REPO_SYNC_APP_KEY")

def validate_environment():
    if REPO_SYNC_APP_ID is None:
        error("The environment variable `REPO_SYNC_APP_ID` is not set! Aborting.")
        sys.exit(3)
    if REPO_SYNC_APP_KEY is None:
        error("The environment variable `REPO_SYNC_APP_KEY` is not set! Aborting.")
        sys.exit(4)


def find_app_installation_by_repo(appId, appKey, repoOwner, repoName):
    """ login as app, and get the app installation for the repository repoName owned by repoOwner"""
    expire_s = 600
    if isinstance(appKey, str):
        appKey = appKey.encode()

    gh = github3.github.GitHub()
    info(f"Trying to login into GH app {appId}")
    gh.login_as_app(appKey, appId, expire_in=expire_s)

    installation = gh.app_installation_for_repository(repoOwner, repoName)

    if installation is None:
        return None

    app = github3.github.GitHub()
    app.login_as_app_installation(appKey, appId, installation.id, expire_in=expire_s)

    return app

def make_token(appId, appKey, url):
    """ create an access token for the repository at url

    Uses github APP authentication to create an app installation access token
    which can be used (if the 'contents' scope is granted for the app) to
    access git via https://x-access-token:<token>@github.com/owner/repo
    """

    #the url is of the form https://github.com/owner/reponame
    parsedUrl = urlparse(url)
    _, repoOwner, repoName = parsedUrl.path.split("/")
    info(f"owner={repoOwner}, repo={repoName}")
    app = find_app_installation_by_repo(appId, appKey, repoOwner, repoName)
    if app is None:
        error(f"finding an authenticated GitHub app installation failed for {url}")
        sys.exit(1)

    repo = app.repository(repoOwner, repoName)
    if repo is None:
        error(f"could not open GH repo {url}")
        sys.exit(1)

    if repo.session.auth.token is None:
        error("repository does not have an auth token")
        sys.exit(2)

    return repo.session.auth.token

def sync_branch(args):
    debug(f"args={args}")
    validate_environment()

    access_token = make_token(REPO_SYNC_APP_ID, REPO_SYNC_APP_KEY, args.REPO_URL)
    auth_url = args.REPO_URL.replace("https://", f"https://x-access-token:{access_token}@")

    info(f"calling `git push --force {args.REPO_URL} {args.LOCAL_BRANCH}:refs/heads/{args.REMOTE_BRANCH}`")
    out = check_output(f"git push --force {auth_url} {args.LOCAL_BRANCH}:refs/heads/{args.REMOTE_BRANCH}", shell=True)
    info(f" ==> git output: {out}")

def publish_release(args):
    print(f"args={args}")
    info("Publish Release not implemented, yet")

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="""Vector GitHub Tool
Uses a GitHub Application for authentication against a remote GitHub.com repository.
Requires environment variables REPO_SYNC_APP_ID with the GitHub application ID
and REPO_SYNC_APP_KEY with a private key retrieved from a GitHub application.
""")
    ap.add_argument('--app-id', type=int,
            help='Provide an Application Id. Otherwise the environment variable REPO_SYNC_APP_ID is required.')
    subparsers = ap.add_subparsers(help='use `sub-command -h` to get detailed help')
    # sub-command to synchronize branches
    parser_syncbranch = subparsers.add_parser('sync-branch',
        help='Synchronize a local branch to a remote repository branch, using GitHub App authentication. see `sync-branch -h` for details.')
    parser_syncbranch.add_argument('REPO_URL',  help='The github.com URL of the target repository.')
    parser_syncbranch.add_argument('LOCAL_BRANCH',  help='The local git branch to synchronize with remote')
    parser_syncbranch.add_argument('REMOTE_BRANCH',  help='The remote git branch to synchronize to')
    parser_syncbranch.set_defaults(func=sync_branch)
    # sub-command to publish releases
    parser_publishrelease = subparsers.add_parser('publish-release',
        help='Create and publish a GitHub release, using GitHub App authentication')
    parser_publishrelease.set_defaults(func=publish_release)


    args = ap.parse_args()

    if args.app_id is not None:
        REPO_SYNC_APP_ID = str(args.app_id)

    if hasattr(args, "func"):
        args.func(args)
    else:
        print("Error: Please specify a sub-command. See `--help` option.")

