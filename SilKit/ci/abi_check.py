#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

"""
Check that a new SIL Kit library is binary compatible with a baseline release: the public integration tests
built from the baseline release are run with the new libSilKit.so in place of the baseline one.
"""

from fnmatch import fnmatchcase
from pathlib import Path

import argparse
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ci_utils import log, info, warn, die

LIBRARY = "libSilKit.so"
TEST_EXECUTABLE = "SilKitIntegrationTests"
SUITE_TIMEOUT_SECONDS = 600
VERSION_LINE = re.compile(r"Running with SIL Kit version '([^']*)' and git hash '([^']*)'")


def exported_symbols(library: Path):
    result = subprocess.run(["nm", "-D", "--defined-only", "--format=posix", str(library)],
                            capture_output=True, text=True, check=True)
    return {line.split()[0] for line in result.stdout.splitlines() if line.strip()}


def read_exclusions(path: Path):
    patterns = []
    for line in path.read_text(encoding="utf-8").splitlines():
        pattern, _, reason = line.partition("#")
        pattern = pattern.strip()
        if not pattern:
            continue
        if not reason.strip():
            die(1, "{}: exclusion '{}' has no reason", path, pattern)
        patterns.append(pattern)
    return patterns


def list_tests(executable: Path, env):
    result = subprocess.run([str(executable), "--gtest_list_tests"], cwd=executable.parent, env=env,
                            capture_output=True, text=True)
    if result.returncode != 0:
        die(1, "Listing the tests failed:\n{}{}", result.stdout, result.stderr)

    suites = {}
    current = None
    for line in result.stdout.splitlines():
        if not line.strip():
            continue
        if line.startswith(" "):
            if current is not None:
                suites[current].append(line.split()[0])
            continue
        name = line.split()[0]
        current = name[:-1] if name.endswith(".") else None
        if current is not None:
            suites[current] = []
    return suites, result.stdout


def run_suite(executable: Path, gtest_filter: str, env):
    try:
        result = subprocess.run([str(executable), f"--gtest_filter={gtest_filter}"], cwd=executable.parent, env=env,
                                capture_output=True, text=True, timeout=SUITE_TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired as ex:
        output = ex.stdout.decode(errors="replace") if ex.stdout else ""
        return False, f"{output}\nTimed out after {SUITE_TIMEOUT_SECONDS} s"
    return result.returncode == 0, result.stdout + result.stderr


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-dir", type=Path, required=True,
                        help=f"Build output directory of the baseline release, containing {TEST_EXECUTABLE} and {LIBRARY}")
    parser.add_argument("--new-lib", type=Path, required=True, help=f"The new {LIBRARY}")
    parser.add_argument("--exclusions", type=Path,
                        help="File with one gtest pattern per line, each followed by '# <reason>'")
    parser.add_argument("--expect-git-hash",
                        help="Git hash the new library must report, proving that the tests run with it")
    args = parser.parse_args()

    baseline_dir = args.baseline_dir.resolve()
    executable = baseline_dir / TEST_EXECUTABLE
    library = baseline_dir / LIBRARY
    baseline_library = baseline_dir / (LIBRARY + ".baseline")
    for path in (executable, library, args.new_lib):
        if not path.is_file():
            die(1, "{} not found", path)

    # keep the original, so the check can be repeated on the same baseline directory
    if not baseline_library.exists():
        shutil.copy2(library, baseline_library)
    shutil.copy2(args.new_lib, library)

    failures = []

    missing = sorted(exported_symbols(baseline_library) - exported_symbols(library))
    if missing:
        warn("{} symbol(s) exported by the baseline library are missing in the new library:\n{}",
             len(missing), "\n".join(missing))
        failures.append("exported symbols")

    # LD_LIBRARY_PATH takes precedence over the build directory in the executable's RUNPATH, so the swapped
    # library is loaded even if the baseline directory was moved. LD_BIND_NOW resolves all symbols at load time,
    # so a missing one fails the whole suite instead of a single call.
    library_path = os.pathsep.join(filter(None, [str(baseline_dir), os.environ.get("LD_LIBRARY_PATH")]))
    env = dict(os.environ, LD_BIND_NOW="1", LD_LIBRARY_PATH=library_path)
    suites, listing = list_tests(executable, env)

    version = VERSION_LINE.search(listing)
    if version is None:
        die(1, "The test executable did not report the SIL Kit version:\n{}", listing)
    info("Baseline tests run with SIL Kit version {} ({})", version.group(1), version.group(2))
    if args.expect_git_hash and version.group(2) != args.expect_git_hash:
        die(1, "Expected the new library with git hash {}, but the tests loaded {}", args.expect_git_hash,
            version.group(2))

    exclusions = read_exclusions(args.exclusions) if args.exclusions else []
    skipped = []
    for suite, tests in sorted(suites.items()):
        names = [f"{suite}.{test}" for test in tests]
        excluded = [pattern for pattern in exclusions if any(fnmatchcase(name, pattern) for name in names)]
        if names and all(any(fnmatchcase(name, pattern) for pattern in excluded) for name in names):
            skipped.append(suite)
            continue

        gtest_filter = f"{suite}.*" + ("-" + ":".join(excluded) if excluded else "")
        passed, output = run_suite(executable, gtest_filter, env)
        log("{} {}", "PASSED" if passed else "FAILED", suite)
        if not passed:
            log("{}", output)
            failures.append(suite)

    log("Skipped (excluded): {}", ", ".join(skipped) if skipped else "none")
    if failures:
        die(1, "ABI check failed: {}", ", ".join(failures))
    info("ABI check passed: {} suite(s) ran with the new library", len(suites) - len(skipped))


if __name__ == "__main__":
    main()
