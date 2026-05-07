# Repository Layout

This page explains how the SIL Kit repository is organized and where to look when making changes.
It is intended as a practical map for developers, not as an exhaustive description of every target.

## Top-Level Overview

The main top-level directories are:

- `SilKit/`: the library itself, including public headers, implementation, and tests
- `Utilities/`: standalone tools such as the registry, monitor, and system controller
- `Demos/`: example applications and demo programs
- `docs/`: Sphinx and Doxygen documentation sources
- `ThirdParty/`: vendored dependencies managed by the build
- `cmake/`: shared top-level CMake helper modules and packaging files
- `.github/`: CI workflows and GitHub project metadata

Common generated directories that should usually be ignored during development:

- `_build/`, `_build*/`: local build trees
- `_install/`, `install*/`: install trees
- `out/`: local output directory
- `.vs/`: Visual Studio workspace state

## Entry Points

The main build entry point is the repository root `CMakeLists.txt`.

That file:

- declares the top-level build options such as `SILKIT_BUILD_TESTS` and `SILKIT_BUILD_DOCS`
- enables testing globally with `enable_testing()`
- includes shared CMake helpers from `cmake/` and `SilKit/cmake/`
- adds the main subprojects: `SilKit/`, `Utilities/`, `Demos/`, and optionally `docs/`

`CMakePresets.json` provides the default local and CI-oriented configure, build, and test presets.

## The `SilKit/` Directory

`SilKit/` contains the actual product library and most of the code developers will touch.

Important subdirectories:

- `SilKit/include/`: public headers installed for consumers of the library
- `SilKit/source/`: implementation code and most internal modules
- `SilKit/IntegrationTests/`: integration and functional tests
- `SilKit/cmake/`: library-specific CMake modules, toolchains, and test helpers
- `SilKit/ci/`: CI scripts and packaging helpers

### `SilKit/source/`

The main implementation is split into several areas:

- `util/`: shared helper code
- `config/`: configuration loading and related support code
- `tracing/`: tracing support
- `core/`: core runtime internals, participant internals, request/reply, and transport-related code
- `services/`: protocol and service implementations such as CAN, Ethernet, LIN, FlexRay, PubSub, RPC, logging, metrics, and orchestration
- `extensions/`: extension-related implementation
- `capi/`: C API implementation layer
- `experimental/`: experimental features, including network simulator internals
- `dashboard/`: dashboard-related client and service code

The source tree builds up many object libraries which are then linked into the final `SilKit` library.
That means changes in a small submodule often affect the final shared library without introducing a new standalone binary.

### Public vs. Internal Code

As a rule of thumb:

- start in `SilKit/include/` when you need to understand the public API shape
- start in `SilKit/source/` when you need to change behavior or internals
- check `SilKit/source/capi/` when a change must preserve or extend C API behavior

The repository also distinguishes between public and internal test coverage through separate test executables.

## Tests

Tests are centered under `SilKit/IntegrationTests/` and are wired up through `SilKit/cmake/SilKitTest.cmake`.

There are several aggregate test executables:

- `SilKitUnitTests`
- `SilKitIntegrationTests`
- `SilKitInternalIntegrationTests`
- `SilKitFunctionalTests`
- `SilKitInternalFunctionalTests`

CTest entries are registered per test suite, not just per executable.
The helper macro extracts suite names and creates one CTest test per suite using `--gtest_filter=<suite>.*`.

Practical consequence:

- `ctest` output is usually more granular than the number of binaries alone suggests
- if you add a new `*Test_*.cpp` file through the helper macro, it will usually appear as its own CTest suite name

## The `Utilities/` Directory

`Utilities/` contains standalone tools built on top of the library:

- `SilKitRegistry/`
- `SilKitSystemController/`
- `SilKitMonitor/`

These are included only when `SILKIT_BUILD_UTILITIES=ON`.
If you are changing startup flows, participant discovery, orchestration, or developer tooling, this directory is often relevant.

## The `Demos/` Directory

`Demos/` contains example applications and sample integrations.

The demos serve two purposes:

- examples for users of the library
- a source of integration-style validation for common flows

Notable areas include:

- `Demos/communication/`: protocol-oriented demos such as CAN, Ethernet, FlexRay, LIN, PubSub, and RPC
- `Demos/api/`: API-oriented examples
- `Demos/tools/Benchmark/`: benchmarking-related demo tooling

Some integration tests in `SilKit/IntegrationTests/` are based on demo applications, so changes in demos can affect test behavior.

## The `docs/` Directory

`docs/` contains the documentation sources used by the Sphinx and Doxygen build.

Key areas:

- `docs/dev-wiki/`: this developer wiki
- `docs/_static/` and `docs/_templates/`: documentation assets and templates

The repository's `docs/` tree contains the separately maintained documentation set, while `docs/dev-wiki/`
acts as the developer-focused knowledge base for repository work.

The documentation build is enabled only when `SILKIT_BUILD_DOCS=ON`.

## The `ThirdParty/` Directory

`ThirdParty/` holds vendored dependencies used by the build.

Treat this directory carefully:

- avoid editing vendored code unless there is a concrete reason
- prefer changes in SIL Kit integration code over patching third-party sources
- if a change must touch `ThirdParty/`, document the reason clearly

## How To Navigate Changes

When working on a change, this starting point usually works well:

1. Find the public API or executable entry point involved.
2. Trace from the relevant `CMakeLists.txt` into the concrete source directory.
3. Identify the owning implementation area under `SilKit/source/`, `Utilities/`, or `Demos/`.
4. Check nearby tests and existing docs before editing.

## Related Pages

- [Build and Test](./build-and-test.md)
- [Developer Wiki Front Page](./README.md)
