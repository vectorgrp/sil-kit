#! /bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import subprocess
import json
import logging
import os
import argparse

logger = logging.getLogger(__name__)
handler = logging.StreamHandler()
logger.addHandler(handler)
logger.setLevel(logging.DEBUG)

from pathlib import Path

def find_VsDevCmd() -> str:

    some_out = subprocess.run(
            ["vswhere", "-latest", "-property", "InstallationPath"],
            capture_output=True, encoding="utf-8", check=True)

    print(some_out.stdout)


    return Path(some_out.stdout.rstrip()) / r'Common7\Tools\VsDevCmd.bat'

def main():

    parser = argparse.ArgumentParser(prog="Microsoft Dev Env Creator",
                                     description="Setup the development environment under Windows")
    parser.add_argument('--arch', type=str, default='x64',
                        help='The architecture for which to build the binary for')
    parser.add_argument('--hostarch', type=str, default='x64',
                        help='The architecture of the host build machine')
    parser.add_argument('--vcvars_ver', type=str, default='14.2',
                        help='The version of the toolset to be used')
    args = parser.parse_args()

    old_env = json.loads(json.dumps(dict(os.environ)))

    vsdevcmd_path = find_VsDevCmd()
    vsdevcmd = f'"{vsdevcmd_path}" -arch={args.arch} -host_arch={args.hostarch} -vcvars_ver={args.vcvars_ver}'

    logger.debug(f"Executing cmd:\n{vsdevcmd}")

    osenvcmd = '( python -c "import os, json; print(json.dumps(dict(os.environ)))" )'
    logger.debug(f"Found at: {vsdevcmd_path}")
    vsdevcmd_proc = subprocess.run(
            executable=r'C:\Windows\System32\cmd.exe',
            args=f'/S /C "( ( {vsdevcmd} >NUL 2>&1 ) && {osenvcmd} ) & exit"',
            capture_output=True, encoding="utf-8", check=False)

    new_env = vsdevcmd_proc.stdout
    if not new_env:
        logger.error("Could not acquire a new shell env!")
        exit(1)

    environment = json.loads(vsdevcmd_proc.stdout)

    with open(os.environ['GITHUB_ENV'], 'a') as f:
        for k, v in environment.items():
            # Ignore CI env set by Github
            if v.startswith(('GITHUB_', 'RUNNER_', 'CI')):
                continue

            # Ignore old values that did not change
            if k in old_env:
                if environment[k] == old_env[k]:
                    continue

            f.write(f"{k}={v}\n")
            logger.debug(f"Setting: {k}={v}")



if __name__ == "__main__":
    main()
