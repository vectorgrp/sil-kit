#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
