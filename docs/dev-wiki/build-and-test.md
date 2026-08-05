# Build and Test

This page summarizes the practical build and test workflows for local development in this repository.

## Prerequisites

At minimum, local development requires:

- Git
- CMake
- a supported C++ toolchain such as Visual Studio, GCC, or Clang
- initialized submodules

Initialize submodules after cloning:

```powershell
git submodule update --init --recursive
```

Documentation builds additionally require:

- Python 3
- dependencies from `SilKit/ci/docker/docs_requirements.txt`
- `pipenv`
- Doxygen

## Preferred Local Flow: CMake Presets

The repository already defines presets in `CMakePresets.json`.
For day-to-day development, presets are the easiest way to stay aligned with the intended build layout.

Useful configure presets include:

- `debug`
- `release`
- `relwithdebinfo`
- `distrib`
- `x86-debug`
- `vs141-x64-debug`

The presets use these default directory conventions:

- build tree: `_build/<preset>`
- install tree: `_install/<preset>`

Typical local debug build:

```powershell
cmake --preset debug
cmake --build --preset debug
```

Typical release build:

```powershell
cmake --preset release
cmake --build --preset release
```

## Important CMake Options

The root `CMakeLists.txt` defines the main switches developers usually care about:

- `SILKIT_BUILD_TESTS`: build unit, integration, and functional tests
- `SILKIT_BUILD_UTILITIES`: build utilities such as registry, monitor, and system controller
- `SILKIT_BUILD_DEMOS`: build demo applications
- `SILKIT_BUILD_DOCS`: build Sphinx and Doxygen documentation
- `SILKIT_INSTALL_SOURCE`: install and package the source tree
- `SILKIT_WARNINGS_AS_ERRORS`: treat compiler warnings as errors
- `SILKIT_BUILD_DASHBOARD`: build the dashboard client code
- `SILKIT_BUILD_STATIC`: build SIL Kit as a static library instead of shared

Most local development can start from the `debug` preset and only override options when needed.

## Manual Configure Flow

If you do not want to use presets, a manual configure still works.

Example:

```powershell
cmake -S . -B _build/manual-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build _build/manual-debug
```

To enable docs explicitly:

```powershell
cmake -S . -B _build/docs -G Ninja -DCMAKE_BUILD_TYPE=Debug -DSILKIT_BUILD_DOCS=ON
cmake --build _build/docs --target Doxygen
```

## Build Outputs

A few output conventions are helpful to know:

- build trees usually live under `_build/`
- presets install into `_install/`
- test executables are configured to run from the build output directory
- demo binaries are emitted into the common build output directory when built in-tree

The library project sets a debug postfix of `d`, so debug binaries can sit alongside release binaries.

## Running Tests

Testing is enabled globally in the root build with `enable_testing()`.
The library creates several aggregate GoogleTest executables, and CTest registers suites through helper macros in `SilKit/cmake/SilKitTest.cmake`.

If you configured with a preset that enables tests, run all tests with:

```powershell
ctest --preset debug --output-on-failure
```

Or from a specific build directory:

```powershell
ctest --test-dir _build/debug --output-on-failure
```

Useful variants:

```powershell
ctest --test-dir _build/debug -R Lin --output-on-failure
ctest --test-dir _build/debug -R PubSub --output-on-failure
ctest --test-dir _build/debug -N
```

What these do:

- `-R <regex>` filters tests by CTest test name
- `-N` lists tests without running them
- `--output-on-failure` prints failing test output immediately

## How Tests Are Structured

The main test executables are:

- `SilKitUnitTests`
- `SilKitIntegrationTests`
- `SilKitInternalIntegrationTests`
- `SilKitFunctionalTests`
- `SilKitInternalFunctionalTests`

Tests are added through `add_silkit_test_to_executable(...)`.
That helper usually creates one CTest entry per test suite, using a GoogleTest filter.

Practical implications:

- CTest test names usually map to suites rather than binary names
- adding a new integration or functional test often means adding a source file in `SilKit/IntegrationTests/` and registering it in the corresponding `CMakeLists.txt`
- if a test seems missing from CTest, check whether it was wired through the helper macro

## Running A Narrow Slice During Development

For fast iteration, prefer a narrow configure and a filtered test run.

Examples:

- build one preset once, then rebuild incrementally with `cmake --build --preset debug`
- run only matching suites with `ctest --test-dir _build/debug -R <name> --output-on-failure`
- list available tests first with `ctest --test-dir _build/debug -N`

This is usually faster and more predictable than repeatedly creating fresh build directories.

## Building Utilities and Demos

Utilities are included when `SILKIT_BUILD_UTILITIES=ON`.
That includes:

- `SilKitRegistry`
- `SilKitSystemController`
- `SilKitMonitor`

Demos are included when `SILKIT_BUILD_DEMOS=ON`.
The `Demos` project contains communication demos, API demos, and benchmark tooling.

For many feature changes, building both tests and utilities is more useful than building demos.
For changes affecting example flows or protocol behavior, demos may also be worth rebuilding.

## Building Documentation

The docs build is optional and uses both Doxygen and Sphinx.

Install dependencies:

```powershell
pip3 install -r SilKit/ci/docker/docs_requirements.txt
pip3 install pipenv
```

Configure and build docs:

```powershell
cmake -S . -B _build/docs -G Ninja -DSILKIT_BUILD_DOCS=ON
cmake --build _build/docs --target Doxygen
```

The docs target named `Doxygen` actually drives both Doxygen extraction and the Sphinx HTML build.

## Packaging

Packaging is handled through CPack.

Typical packaging command after configuration:

```powershell
cmake --build _build/distrib --target package
```

The `distrib` preset is the closest existing preset to a packaging-oriented build.

## Practical Recommendations

For everyday code changes:

1. Use `cmake --preset debug` once.
2. Rebuild incrementally with `cmake --build --preset debug`.
3. Run only the relevant CTest slice while iterating.
4. Run a broader `ctest --preset debug --output-on-failure` before finishing.

For documentation changes:

1. Enable `SILKIT_BUILD_DOCS`.
2. Build the `Doxygen` target.
3. Check the generated Sphinx output and warnings.

For release-like validation:

1. Use the `distrib` preset.
2. Build and test from that preset.
3. Run the `package` target if needed.

## Related Pages

- [Repository Layout](./repository-layout.md)
- [Developer Wiki Front Page](./README.md)
