# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

import dataclasses
import tomllib
import os
import subprocess
import signal
import csv
import argparse
import typing
import platform

from git import Repo  # PyPI: GitPython

SCRIPT_PATH = os.path.abspath(__file__)
WINDOWS = platform.system() == "Windows"


# data structures

@dataclasses.dataclass
class Config:
    tests: list['Test']
    repositories: 'ConfigRepositories'
    verbose: bool | None = None
    work_dir: str | None = None

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
        return TestRun(csv_output=os.path.join(repository.results_dir, test.csv_output))


##### function definitions #####

def get_command(command, bin_dir):
    if WINDOWS:
        command = f"{command}.exe"
    return os.path.join(bin_dir, command)


def spawn(args: list[str], bin_dir: str, verbose: bool) -> subprocess.Popen:
    args = [get_command(args[0], bin_dir)] + args[1:]
    output = subprocess.DEVNULL

    if verbose:
        output = None
        print(f"spawning: {args!r}")

    popen = subprocess.Popen(args, stdout=output, stderr=output)
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

    # clone from GitHub
    url = "https://github.com/vectorgrp/sil-kit.git"

    print(f"Cloning {url!r} into {source_dir!r}")
    repo = Repo.clone_from(url, source_dir)

    print(f"Checking out version {version!r}")
    repo.git.checkout(version)

    print(f"Updating submodules")
    output = repo.git.submodule('update', '--init', '--recursive')
    print(output)


def configure(repository: 'ConfigRepository'):
    source_dir = repository.source_dir
    build_dir = repository.build_dir

    if repository.skip_configure:
        print("Skipping configure as requested")
        return

    if os.path.isdir(build_dir):
        print(f"Skipping configure because the directory {build_dir!r} already exists")
        return

    run(['cmake', f"-S{source_dir}", f"-B{build_dir}", "-DCMAKE_BUILD_TYPE=Release", "-DSILKIT_BUILD_TESTS=OFF"])


def build(repository: 'ConfigRepository'):
    build_dir = repository.build_dir

    if repository.skip_build:
        print("Skipping build as requested")
        return

    run(['cmake', "--build", build_dir, "--config", "Release", "--parallel", "--target", "sil-kit-registry"])
    run(['cmake', "--build", build_dir, "--config", "Release", "--parallel", "--target", "SilKitDemoBenchmark"])
    run(['cmake', "--build", build_dir, "--config", "Release", "--parallel", "--target", "SilKitDemoLatency"])


def start_registry(repository: 'ConfigRepository', config: 'Config'):
    return spawn(["sil-kit-registry", "--log", "off"], repository.bin_dir, config.verbose).pid


def kill_process(pid: int):
    os.kill(pid, signal.SIGTERM)


def prepare_repository(repository: ConfigRepository, force: bool):
    results_dir = repository.results_dir

    if not force and os.path.exists(results_dir):
        # TODO If something went wrong in creating the ref. KPIs (e.g. registry collision, build failure,...), the folder exists but the result files not
        print(f"Skipping repository preparation because directory {results_dir!r} already exists")
        return

    clone(repository)
    configure(repository)
    build(repository)


def run_process(process: Process, config: Config, bin_dir: str, **kwargs) -> subprocess.Popen:
    format_arg = lambda f: f.format(process=process, config=config, **kwargs)
    args = [format_arg(arg) for arg in [process.executable] + process.args]
    return spawn(args, bin_dir, config.verbose)


def run_test(test: Test, repository: ConfigRepository, config: Config):
    if not test.enabled:
        print(f"Skipping test {test.name!r} as configured")
        return

    if os.path.isfile(test.csv_output):
        os.remove(test.csv_output)

    demo_processes = []

    test_run = TestRun.new(test, repository)

    for demo in test.demos:
        popen = run_process(demo, config, repository.bin_dir, test=test, run=test_run)
        demo_processes.append(popen)

    for process in demo_processes:
        process.communicate()


def run_tests(repository: ConfigRepository, config: Config, force: bool):
    results_dir = repository.results_dir

    if not force and os.path.exists(results_dir):
        # TODO If something went wrong in creating the ref. KPIs (e.g. registry collision, build failure,...), the folder exists but the result files not
        print(f"Skipping test execution because directory {results_dir!r} already exists")
        return

    os.makedirs(results_dir, exist_ok=True)

    sil_kit_registry_pid = start_registry(repository, config)

    for test in config.tests:
        run_test(test, repository, config)

    kill_process(sil_kit_registry_pid)


def read_kpi(path: str, kpi_label: str):
    with open(path) as csv_file:
        lines = csv_file.readlines()[1:]  # skip comment
        data = csv.DictReader(lines, delimiter=';', skipinitialspace=True)
        vals = [float(row[kpi_label]) for row in data]
        kpi_value = vals[-1]

    return kpi_value


def assess_test(test: Test, reference: ConfigRepository, under_test: ConfigRepository):
    if not test.enabled:
        print(f"Skipping assessment of test {test.name!r} as configured")
        return

    # get reference kpi values
    reference_test_run = TestRun.new(test, reference)
    reference_mean = read_kpi(reference_test_run.csv_output, test.kpis.mean.label)
    reference_err = read_kpi(reference_test_run.csv_output, test.kpis.err.label)

    # compute thresholds (2 sigma rule)
    sigma = 2.0
    reference_upper_threshold = reference_mean + sigma * reference_err
    reference_lower_threshold = reference_mean - sigma * reference_err

    # get under-test kpi values
    under_test_test_run = TestRun.new(test, under_test)
    under_test_mean = read_kpi(under_test_test_run.csv_output, test.kpis.mean.label)
    under_test_err = read_kpi(under_test_test_run.csv_output, test.kpis.err.label)

    # check if standard deviation is larger than 10% of the mean (NB: 10% is heuristically chosen and may be adapted in the future)
    err_coeff = 0.1
    warn = str("")
    if (under_test_mean * err_coeff) < under_test_err:
        warn = f" [WARNING: Standard deviation is larger than {err_coeff:.0%} of the mean.]"

    def report(topic: str, passed: bool, extra: str, optionalWarning: str):
        print(f"{topic + ': ':<30}{'PASSED' if passed else 'FAILED'}{extra}{optionalWarning}")

    report(
        test.topic,
        reference_lower_threshold < under_test_mean < reference_upper_threshold,
        f" with {under_test_mean} {test.unit} (acceptance interval: {reference_lower_threshold} - {reference_upper_threshold} {test.unit})",
        warn
    )    

def assess_kpis(reference: ConfigRepository, under_test: ConfigRepository, config: Config):
    print("\n" + "----- Test Report (start) -----" + "\n")

    for test in config.tests:
        assess_test(test, reference, under_test)

    print("\n" + "----- Test Report (end) -------")


##### start script #####

DEFAULT_REFERENCE_VERSION = "v4.0.52"

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


def update_config(config: Config, args: object):
    override_or(config, args, "work_dir", os.path.join(os.getcwd(), "_work"))
    override_or(config, args, "verbose", False)

    config.work_dir = os.path.abspath("_work")

    for name, repository in vars(config.repositories).items():
        override_with_or(repository, "source_dir", None, os.path.join(config.work_dir, "s", name))
        override_with_or(repository, "build_dir", None, os.path.join(config.work_dir, "b", name))
        override_with_or(repository, "bin_dir", None, os.path.join(repository.build_dir, "Release"))
        override_with_or(repository, "results_dir", None, os.path.join(config.work_dir, "r", name))


def main():
    parser = argparse.ArgumentParser(description="Process a reference tag or commit id.")
    parser.add_argument('--reference-version', type=str, default=None,
                        help='Reference tag or commit id of the reference version')
    parser.add_argument('--version-under-test', type=str, default=None,
                        help='Reference tag or commit id for the version under test')
    parser.add_argument('--work-dir', type=str, default=None)
    parser.add_argument('-v', '--verbose', action='store_true', default=None,
                        help='Print output of SIL Kit applications to stdout')
    args = parser.parse_args()

    with open(os.path.join(os.path.dirname(SCRIPT_PATH), "performance-tests.toml"), "rb") as f:
        config = Config(**tomllib.load(f))

    update_config(config, args)

    prepare_repository(config.repositories.reference, force=False)
    prepare_repository(config.repositories.under_test, force=True)

    run_tests(config.repositories.reference, config, force=False)
    run_tests(config.repositories.under_test, config, force=True)

    assess_kpis(config.repositories.reference, config.repositories.under_test, config)


if __name__ == "__main__":
    main()
