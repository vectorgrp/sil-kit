# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT
import platform
import sys

min_python_version = (3, 11, 0)
current_python_version = tuple(map(int, platform.python_version_tuple()))
if current_python_version < min_python_version:
    raise RuntimeError("The minimum Python version needed is " +
                       ".".join(str(c) for c in min_python_version))

import dataclasses
import tomllib
import os
import subprocess
import signal
import csv
import argparse
import typing
import shutil
import time
import glob

SCRIPT_PATH = os.path.abspath(__file__)
WINDOWS = platform.system() == "Windows"

# Prefer Ninja when it is available. The platform default is Visual Studio on Windows and
# Makefiles elsewhere, both of which build far slower than Ninja for a from-scratch build, and this
# script does two of those per run.
USE_NINJA = shutil.which("ninja") is not None

# NB: binaries live in a per-configuration subdirectory regardless of the generator, because the
#     project pins RUNTIME_OUTPUT_DIRECTORY to ${CMAKE_BINARY_DIR}/$<CONFIG>. Do not "fix" bin_dir
#     to drop the Release component when building with a single-config generator such as Ninja.

# Configure arguments applied to every build.
#
# The dashboard is not exercised by any benchmark, so building it only costs time. Disabling it
# also keeps oatpp out of the build entirely, which matters for revisions before 5.0.8 where oatpp
# was still vendored: its CMakeLists declares a minimum below 3.5, which CMake 4 refuses outright.
#
# CMAKE_POLICY_VERSION_MINIMUM is a belt-and-braces fallback for any other old vendored dependency.
# SIL Kit itself has required at least 3.12 across all versions this script compares, so this can
# never mask a policy problem in SIL Kit's own CMake files.
DEFAULT_CMAKE_CONFIGURE_ARGS = [
    "-DSILKIT_BUILD_DASHBOARD=OFF",
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
]

REPOSITORY_URL = "https://github.com/vectorgrp/sil-kit.git"

BUILD_TARGETS = ["sil-kit-registry", "SilKitDemoBenchmark", "SilKitDemoLatency"]

# Identifies this invocation. Result files are prefixed with it so that successive runs
# accumulate in the results directory instead of overwriting each other.
RUN_ID = time.strftime("%Y%m%d-%H%M%S")

# How often to report that a demo is still running. The demos can run for many minutes without
# producing output, so without this there is no way to tell a slow run from a hung one.
PROGRESS_INTERVAL_SECONDS = 30


# data structures

@dataclasses.dataclass
class Config:
    tests: list['Test']
    repositories: 'ConfigRepositories'
    verbose: bool | None = None
    work_dir: str | None = None
    cmake_configure_arg: list[str] | None = None

    def __post_init__(self):
        self.repositories = ConfigRepositories(**typing.cast(dict, self.repositories))
        self.tests = [Test(**d) for d in typing.cast(list[dict], self.tests)]


@dataclasses.dataclass
class ConfigRepositories:
    reference: 'ConfigRepository'
    under_test: 'ConfigRepository'

    def __post_init__(self):
        self.reference = ConfigRepository(**typing.cast(dict, self.reference))
        self.under_test = ConfigRepository(**typing.cast(dict, self.under_test))


@dataclasses.dataclass
class ConfigRepository:
    version: str
    # Extra configure arguments for this repository only, e.g. to accommodate an old reference
    # revision. Applied after the defaults, so they win.
    cmake_configure_arg: list[str] | None = None
    skip_clone: bool = False
    skip_configure: bool = False
    skip_build: bool = False
    source_dir: str | None = None
    build_dir: str | None = None
    bin_dir: str | None = None
    results_dir: str | None = None


@dataclasses.dataclass
class Test:
    name: str
    topic: str
    unit: str
    kpis: 'TestKpis'
    demos: ['Process']
    csv_output: str
    enabled: bool = True
    # Which direction counts as an improvement. Throughput and message rates improve upwards,
    # latencies improve downwards. Used to make the acceptance threshold one-sided.
    higher_is_better: bool = True

    def __post_init__(self):
        self.kpis = TestKpis(**typing.cast(dict, self.kpis))
        self.demos = [Process(**d) for d in typing.cast(list[dict], self.demos)]


@dataclasses.dataclass
class TestKpis:
    mean: 'TestKpisEntry'
    err: 'TestKpisEntry'

    def __post_init__(self):
        self.mean = TestKpisEntry(**typing.cast(dict, self.mean))
        self.err = TestKpisEntry(**typing.cast(dict, self.err))


@dataclasses.dataclass
class TestKpisEntry:
    label: str


@dataclasses.dataclass
class Process:
    executable: str
    args: list[str]


@dataclasses.dataclass
class TestRun:
    csv_output: str

    @staticmethod
    def new(test: Test, repository: ConfigRepository) -> 'TestRun':
        """Where this invocation writes the results of a test."""
        return TestRun(csv_output=os.path.join(repository.results_dir,
                                               f"{RUN_ID}_{test.csv_output}"))


##### function definitions #####

def get_command(command, bin_dir):
    if WINDOWS:
        command = f"{command}.exe"
    return os.path.join(bin_dir, command)


def spawn(args: list[str], bin_dir: str, verbose: bool) -> subprocess.Popen:
    args = [get_command(args[0], bin_dir)] + args[1:]

    if verbose:
        print(f"spawning: {args!r}")

    # NB: the child inherits stdout/stderr rather than having it sent to DEVNULL, so that demo
    #     progress is visible while the run is in flight. Inheriting rather than piping matters:
    #     a pipe would hold the output until the child exits, which is exactly when it stops being
    #     useful.
    popen = subprocess.Popen(args)
    return popen


def run(args: list[str], cwd: str | None = None, check: bool = True):
    print(f"running: {args!r}")
    subprocess.run(args, cwd=cwd, check=check)


def clone(repository: 'ConfigRepository'):
    source_dir = repository.source_dir
    version = repository.version

    if repository.skip_clone:
        print(f"Skipping cloning into {source_dir!r} as configured")
        return

    if os.path.isdir(source_dir):
        print(f"Skipping cloning because the directory {source_dir!r} already exists")
        return

    url = REPOSITORY_URL

    print(f"Cloning {url!r} into {source_dir!r}")
    subprocess.run(["git", "clone", str(url), str(source_dir)])

    print(f"Checking out version {version!r}")
    subprocess.run(["git", "-C", source_dir, "checkout", str(version)])

    print(f"Updating submodules")
    subprocess.run(["git", "-C", source_dir, "submodule", "update", "--init", "--recursive"])


def configure(config: Config, repository: 'ConfigRepository'):
    source_dir = repository.source_dir
    build_dir = repository.build_dir

    if repository.skip_configure:
        print("Skipping configure as requested")
        return

    if os.path.isdir(build_dir):
        print(f"Skipping configure because the directory {build_dir!r} already exists")
        return

    cmd = ["cmake", f"-S{source_dir}", f"-B{build_dir}"]
    if USE_NINJA:
        cmd += ["-GNinja"]
    cmd += ["-DCMAKE_BUILD_TYPE=Release", "-DSILKIT_BUILD_TESTS=OFF"]
    cmd += DEFAULT_CMAKE_CONFIGURE_ARGS
    cmd += config.cmake_configure_arg
    cmd += repository.cmake_configure_arg or []
    run(cmd)


def build(repository: 'ConfigRepository'):
    build_dir = repository.build_dir

    if repository.skip_build:
        print("Skipping build as requested")
        return

    for target in BUILD_TARGETS:
        run(['cmake', "--build", build_dir, "--config", "Release", "--parallel", "--target", target])


def start_registry(repository: 'ConfigRepository', config: 'Config'):
    return spawn(["sil-kit-registry", "--log", "off"], repository.bin_dir, config.verbose).pid


def kill_process(pid: int):
    os.kill(pid, signal.SIGTERM)


def prepare_repository(config: Config, repository: ConfigRepository, force: bool):
    results_dir = repository.results_dir

    if not force and os.path.exists(results_dir):
        # TODO If something went wrong in creating the ref. KPIs (e.g. registry collision, build failure,...), the folder exists but the result files not
        print(f"Skipping repository preparation because directory {results_dir!r} already exists")
        return

    clone(repository)
    configure(config, repository)
    build(repository)


def run_process(process: Process, config: Config, bin_dir: str, **kwargs) -> subprocess.Popen:
    format_arg = lambda f: f.format(process=process, config=config, **kwargs)
    args = [format_arg(arg) for arg in [process.executable] + process.args]
    return spawn(args, bin_dir, config.verbose)


def wait_for_process(process: subprocess.Popen, label: str) -> int:
    """Wait for a demo, reporting periodically so a hung run is distinguishable from a slow one."""
    started = time.monotonic()

    while True:
        try:
            process.wait(timeout=PROGRESS_INTERVAL_SECONDS)
            break
        except subprocess.TimeoutExpired:
            print(f"  [{label}] still running after {int(time.monotonic() - started)}s "
                  f"(pid {process.pid})", flush=True)

    elapsed = time.monotonic() - started

    if process.returncode != 0:
        # NB: report this here rather than letting it surface later as a missing CSV file.
        print(f"  [{label}] WARNING: exited with code {process.returncode} after {elapsed:.1f}s; "
              f"its results will be missing", flush=True)
    else:
        print(f"  [{label}] done after {elapsed:.1f}s", flush=True)

    return process.returncode


def run_test(test: Test, repository: ConfigRepository, config: Config):
    if not test.enabled:
        print(f"Skipping test {test.name!r} as configured")
        return

    demo_processes = []

    test_run = TestRun.new(test, repository)

    for demo in test.demos:
        print(f"running test {test.name!r} with demo {demo.executable!r} from {repository.source_dir!r} ({repository.bin_dir!r})")
        popen = run_process(demo, config, repository.bin_dir, test=test, run=test_run)
        demo_processes.append(popen)

    for demo, process in zip(test.demos, demo_processes):
        wait_for_process(process, f"{test.name}/{demo.executable}")


def missing_results(repository: ConfigRepository, config: Config) -> list[str]:
    """Names of enabled tests that have no result file for this repository."""
    return [test.name for test in config.tests if test.enabled
            and latest_result(repository, test) is None]


def run_tests(repository: ConfigRepository, config: Config, force: bool):
    results_dir = repository.results_dir

    if not force and os.path.exists(results_dir):
        # NB: the directory existing is not enough. An aborted earlier run leaves partial results
        #     behind, and reusing them silently compares against whatever configuration produced
        #     them. Only skip when every enabled test actually has a result.
        missing = missing_results(repository, config)
        if not missing:
            print(f"Skipping test execution because {results_dir!r} already has results for "
                  f"every enabled test")
            return

        print(f"WARNING: {results_dir!r} exists but has no results for {', '.join(missing)}. "
              f"Re-running all tests for this repository, because partial results from an earlier "
              f"run may have used a different configuration.", flush=True)

    os.makedirs(results_dir, exist_ok=True)

    sil_kit_registry_pid = start_registry(repository, config)

    for test in config.tests:
        run_test(test, repository, config)

    kill_process(sil_kit_registry_pid)


def latest_result(repository: ConfigRepository, test: Test) -> str | None:
    """The most recent result file for a test, across all runs, or None if there is none.

    Result files are prefixed with a sortable run id, so the lexicographically last match is the
    newest. The reference results are often produced by an earlier invocation than the version
    under test, so assessment has to look these up rather than assume the current run id.
    """
    matches = glob.glob(os.path.join(repository.results_dir, f"*_{test.csv_output}"))
    return sorted(matches)[-1] if matches else None


def read_last_row(path: str) -> dict:
    """Return the last data row of a result CSV."""
    with open(path) as csv_file:
        lines = csv_file.readlines()[1:]  # skip comment
        rows = list(csv.DictReader(lines, delimiter=';', skipinitialspace=True))

    return rows[-1]


# Columns that describe the workload rather than its result. Both sides of a comparison have to
# agree on these, otherwise the two runs did not measure the same thing.
WORKLOAD_COLUMNS = [
    "participants",
    "messageSize",
    "messageCount",
    "duration(virtual time, s)",
    "stepSize(virtual time, ms)",
]


@dataclasses.dataclass
class Assessment:
    name: str
    topic: str
    unit: str
    verdict: str
    reference_mean: float | None = None
    under_test_mean: float | None = None
    noisy: bool = False

    @property
    def regressed(self) -> bool:
        return self.verdict not in ("PASSED", "IMPROVED", "SKIPPED")

    @property
    def change(self) -> str:
        if not self.reference_mean or self.under_test_mean is None:
            return "-"
        return f"{(self.under_test_mean - self.reference_mean) / self.reference_mean * 100:+.1f}%"


def assess_test(test: Test, reference: ConfigRepository, under_test: ConfigRepository) -> Assessment:
    """Assess one test."""
    if not test.enabled:
        print(f"Skipping assessment of test {test.name!r} as configured")
        return Assessment(test.name, test.topic, test.unit, "SKIPPED")

    reference_path = latest_result(reference, test)
    under_test_path = latest_result(under_test, test)

    # NB: a missing result file used to raise straight out of the report. Report it as a failure of
    #     this test instead, so the remaining tests are still assessed.
    for role, path, repository in (("reference", reference_path, reference),
                                   ("under test", under_test_path, under_test)):
        if path is None:
            print(f"{test.topic + ': ':<30}NO RESULT, no {role} results for {test.name!r} in "
                  f"{repository.results_dir}. Re-run to generate them.")
            return Assessment(test.name, test.topic, test.unit, "NO RESULT")

    reference_row = read_last_row(reference_path)
    under_test_row = read_last_row(under_test_path)

    # NB: results left over from an earlier run can silently be compared against a different
    #     configuration, which shows up as a large but entirely fictitious improvement. Catch it.
    mismatched = [column for column in WORKLOAD_COLUMNS
                  if column in reference_row and column in under_test_row
                  and reference_row[column] != under_test_row[column]]
    if mismatched:
        differences = ", ".join(f"{column}: reference {reference_row[column]} vs under test "
                                f"{under_test_row[column]}" for column in mismatched)
        print(f"{test.topic + ': ':<30}NOT COMPARABLE, the two runs used different parameters "
              f"({differences}). The stale results are in {os.path.basename(reference_path)} and "
              f"{os.path.basename(under_test_path)}; re-run both sides.")
        return Assessment(test.name, test.topic, test.unit, "NOT COMPARABLE")

    reference_mean = float(reference_row[test.kpis.mean.label])
    reference_err = float(reference_row[test.kpis.err.label])

    # compute thresholds (2 sigma rule)
    sigma = 2.0
    reference_upper_threshold = reference_mean + sigma * reference_err
    reference_lower_threshold = reference_mean - sigma * reference_err

    under_test_mean = float(under_test_row[test.kpis.mean.label])
    under_test_err = float(under_test_row[test.kpis.err.label])

    # check if standard deviation is larger than 10% of the mean (NB: 10% is heuristically chosen and may be adapted in the future)
    err_coeff = 0.1
    warn = str("")
    if (under_test_mean * err_coeff) < under_test_err:
        warn = f" [WARNING: Standard deviation is larger than {err_coeff:.0%} of the mean.]"

    # NB: the threshold is one-sided, in the direction that counts as a regression. A two-sided
    #     interval reports a genuine improvement as a failure, which makes the gate unusable.
    if test.higher_is_better:
        regressed = under_test_mean < reference_lower_threshold
        improved = under_test_mean > reference_upper_threshold
        bound = f"acceptance threshold: >= {reference_lower_threshold} {test.unit}"
    else:
        regressed = under_test_mean > reference_upper_threshold
        improved = under_test_mean < reference_lower_threshold
        bound = f"acceptance threshold: <= {reference_upper_threshold} {test.unit}"

    if regressed:
        verdict = "FAILED"
    elif improved:
        verdict = "IMPROVED"
    else:
        verdict = "PASSED"

    print(f"{test.topic + ': ':<30}{verdict} with {under_test_mean} {test.unit} "
          f"(reference: {reference_mean} +/- {reference_err}, {bound}){warn}")

    return Assessment(test.name, test.topic, test.unit, verdict,
                      reference_mean, under_test_mean, noisy=bool(warn))

def print_summary_table(assessments: list[Assessment]):
    """A compact overview of every test, so the outcome is readable at a glance."""
    rows = [("Test", "Unit", "Reference", "Under test", "Change", "Verdict")]
    for a in assessments:
        rows.append((
            a.topic,
            a.unit,
            "-" if a.reference_mean is None else f"{a.reference_mean:.6g}",
            "-" if a.under_test_mean is None else f"{a.under_test_mean:.6g}",
            a.change,
            a.verdict + (" (noisy)" if a.noisy else ""),
        ))

    widths = [max(len(row[i]) for row in rows) for i in range(len(rows[0]))]
    separator = "  ".join("-" * w for w in widths)

    def emit(row):
        # right align the numeric columns, left align the rest
        print("  ".join(cell.rjust(widths[i]) if i in (2, 3, 4) else cell.ljust(widths[i])
                        for i, cell in enumerate(row)).rstrip())

    print()
    emit(rows[0])
    print(separator)
    for row in rows[1:]:
        emit(row)
    print()
    print("Change is the raw difference from the reference. Whether that is good or bad depends "
          "on the test, so read it together with the verdict.")


def assess_kpis(reference: ConfigRepository, under_test: ConfigRepository, config: Config) -> bool:
    """Assess every test. Returns True if none of them regressed."""
    print("\n" + "----- Test Report (start) -----" + "\n")

    assessments = [assess_test(test, reference, under_test) for test in config.tests]

    print_summary_table(assessments)

    print("\n" + "----- Test Report (end) -------")

    regressions = [a.name for a in assessments if a.regressed]
    if regressions:
        print(f"Regressions detected in: {', '.join(regressions)}")

    return not regressions


##### start script #####

DEFAULT_REFERENCE_VERSION = "v5.0.7"

T = typing.TypeVar("T")
U = typing.TypeVar("U")


def override_or(obj: T, args: object, key: str, default=None):
    return override_with_or(obj, key, getattr(args, key), default)


def override_with_or(obj: T, key: str, value: U | None, default=U | None):
    if value is not None:
        setattr(obj, key, value)
        return

    if getattr(obj, key) is None:
        assert default is not None
        setattr(obj, key, default)


def use_prebuilt_binaries(repository: ConfigRepository, bin_dir: str | None, role: str):
    """Point a repository at already built binaries, skipping clone, configure and build."""
    if bin_dir is None:
        return

    repository.bin_dir = os.path.abspath(bin_dir)
    repository.skip_clone = True
    repository.skip_configure = True
    repository.skip_build = True
    repository.version = f"prebuilt binaries in {repository.bin_dir}"

    if not os.path.isdir(repository.bin_dir):
        raise SystemExit(f"--{role}-bin-dir: {repository.bin_dir!r} is not a directory")

    missing = [target for target in BUILD_TARGETS
               if not os.path.isfile(get_command(target, repository.bin_dir))]
    if missing:
        raise SystemExit(f"--{role}-bin-dir: {repository.bin_dir!r} does not contain "
                         f"{', '.join(missing)}")


def update_config(config: Config, args: object):
    override_or(config, args, "work_dir", os.path.abspath("_work"))
    override_or(config, args, "verbose", False)
    override_or(config, args, "cmake_configure_arg", [])

    # NB: these two were previously declared as command line options but never applied, so
    #     passing them silently did nothing and the configured versions were used regardless.
    if args.reference_version is not None:
        config.repositories.reference.version = args.reference_version
    if args.version_under_test is not None:
        config.repositories.under_test.version = args.version_under_test

    for name, repository in vars(config.repositories).items():
        override_with_or(repository, "source_dir", None, os.path.join(config.work_dir, "source", name))
        override_with_or(repository, "build_dir", None, os.path.join(config.work_dir, "build", name))
        override_with_or(repository, "bin_dir", None, os.path.join(repository.build_dir, "Release"))
        override_with_or(repository, "results_dir", None, os.path.join(config.work_dir, "results", name))

    use_prebuilt_binaries(config.repositories.reference, args.reference_bin_dir, "reference")
    use_prebuilt_binaries(config.repositories.under_test, args.under_test_bin_dir, "under-test")


def format_demo_args(demo: Process, test: Test, config: Config, repository: ConfigRepository) -> list[str]:
    """Resolve a demo command line the same way run_process does, for reporting."""
    test_run = TestRun.new(test, repository)
    format_arg = lambda f: f.format(process=demo, config=config, test=test, run=test_run)
    return [format_arg(arg) for arg in [demo.executable] + demo.args]


def print_plan(config: Config):
    """Summarize what will be fetched, built and run before any of it happens."""
    repositories = list(vars(config.repositories).items())

    print("=" * 78)
    print("Performance test plan")
    print("=" * 78)
    print(f"work directory   : {config.work_dir}")
    print(f"cmake generator  : {'Ninja' if USE_NINJA else 'platform default'}")
    print(f"configure args   : {' '.join(DEFAULT_CMAKE_CONFIGURE_ARGS + config.cmake_configure_arg)}")
    print(f"build targets    : {', '.join(BUILD_TARGETS)}")
    print(f"progress         : a heartbeat is printed every {PROGRESS_INTERVAL_SECONDS}s while a demo runs")

    print()
    print("Fetch and build")
    print("-" * 78)
    for name, repository in repositories:
        print(f"  {name} @ {repository.version}")

        if repository.skip_clone:
            fetch = "skipped, disabled in configuration"
        elif os.path.isdir(repository.source_dir):
            fetch = f"skipped, {repository.source_dir} already exists"
        else:
            fetch = f"clone {REPOSITORY_URL}, checkout {repository.version}, init submodules"
        print(f"    fetch     : {fetch}")

        if repository.skip_configure:
            cfg = "skipped, disabled in configuration"
        elif os.path.isdir(repository.build_dir):
            cfg = f"skipped, {repository.build_dir} already exists"
        else:
            cfg = "configure"
        print(f"    configure : {cfg}")
        print(f"    build     : {'skipped, disabled in configuration' if repository.skip_build else 'build'}")
        print(f"    source    : {repository.source_dir}")
        print(f"    binaries  : {repository.bin_dir}")
        print(f"    results   : {repository.results_dir}")

    enabled = [test for test in config.tests if test.enabled]
    skipped = [test.name for test in config.tests if not test.enabled]

    print()
    print(f"Run ({len(enabled)} tests x {len(repositories)} repositories)")
    print("-" * 78)

    # NB: the command lines are resolved against the first repository. Only the CSV output path
    #     differs between repositories.
    reporting_repository = repositories[0][1]

    for test in enabled:
        print(f"  {test.name}  ({test.topic}, {test.unit}, "
              f"{'higher' if test.higher_is_better else 'lower'} is better)")
        for demo in test.demos:
            print(f"    {' '.join(format_demo_args(demo, test, config, reporting_repository))}")

    if skipped:
        print(f"  skipped (disabled in configuration): {', '.join(skipped)}")

    print("=" * 78)
    print(flush=True)


def main():
    parser = argparse.ArgumentParser(description="Process a reference tag or commit id.")
    parser.add_argument('--reference-version', type=str, default=None,
                        help='Reference tag or commit id of the reference version')
    parser.add_argument('--version-under-test', type=str, default=None,
                        help='Reference tag or commit id for the version under test')
    parser.add_argument('--reference-bin-dir', type=str, default=None,
                        help='Use prebuilt binaries from this directory as the reference, '
                             'skipping clone, configure and build')
    parser.add_argument('--under-test-bin-dir', type=str, default=None,
                        help='Use prebuilt binaries from this directory as the version under '
                             'test, skipping clone, configure and build. For example the Release '
                             'directory of a local build')
    parser.add_argument('--work-dir', type=str, default=None)
    parser.add_argument('--cmake-configure-arg', action='append',
                        help='Additional CMake configure argument')
    parser.add_argument('-v', '--verbose', action='store_true', default=None,
                        help='Print output of SIL Kit applications to stdout')
    args = parser.parse_args()

    with open(os.path.join(os.path.dirname(SCRIPT_PATH), "performance-tests.toml"), "rb") as f:
        config = Config(**tomllib.load(f))

    update_config(config, args)

    print_plan(config)

    prepare_repository(config, config.repositories.reference, force=False)
    prepare_repository(config, config.repositories.under_test, force=True)

    run_tests(config.repositories.reference, config, force=False)
    run_tests(config.repositories.under_test, config, force=True)

    return assess_kpis(config.repositories.reference, config.repositories.under_test, config)


if __name__ == "__main__":
    # NB: exit non-zero on a regression so that this can actually gate.
    sys.exit(0 if main() else 1)
