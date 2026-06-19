# Repo Context Hints

## Snapshot

SIL Kit is a C++ library for connecting Software-in-the-Loop environments.

- Build system: `CMake`
- Main language: `C++`
- Start here for overview: `README.rst`
- Start here for architecture and platform support: `docs/for-developers/developers.rst`

## Important Directories

- `SilKit/`: library headers, implementation, packaging, tests
- `Utilities/`: runtime tools like `sil-kit-registry`, `sil-kit-system-controller`, `sil-kit-monitor`
- `Demos/`: example applications
- `docs/`: Sphinx/Doxygen docs and code samples
- `cmake/`: shared CMake modules
- `.github/workflows/`: CI behavior
- `ThirdParty/`: vendored dependencies, usually not the place to edit

## Library Layout

Inside `SilKit/` the most relevant locations are:

- `include/`: public API
- `source/core/`: runtime internals, participant, transport, requests
- `source/services/`: CAN, Ethernet, FlexRay, LIN, PubSub, RPC, logging, metrics, orchestration
- `source/tracing/`: tracing and replay
- `source/experimental/`: experimental APIs and network simulation
- `IntegrationTests/`: integration coverage
- `cmake/SilKitTest.cmake`: test registration helpers

The library is built from many object libraries under `SilKit/source/` and exported as `SilKit::SilKit`.

## Build Defaults

Root `CMakeLists.txt` defaults:

- `SILKIT_BUILD_TESTS=ON`
- `SILKIT_BUILD_UTILITIES=ON`
- `SILKIT_BUILD_DEMOS=ON`
- `SILKIT_BUILD_DASHBOARD=ON`
- `SILKIT_BUILD_DOCS=OFF`
- `SILKIT_BUILD_STATIC=OFF`

Useful presets from `CMakePresets.json`:

- `debug`
- `release`
- `relwithdebinfo`
- `x86-debug`
- `x86-release`
- `distrib`

Common commands:

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

Windows CI uses Ninja and MSVC toolset compatibility matters.

## Tests

- Tests use `CTest` + `GoogleTest`
- Test executables are declared in `SilKit/CMakeLists.txt`
- Test suite registration happens in `SilKit/cmake/SilKitTest.cmake`
- Test source files commonly follow `Test_*.cpp`

When adding tests, prefer the existing `add_silkit_test_to_executable(...)` flow.

## Docs And Runtime

- Docs are built from `docs/` with Doxygen + Sphinx when `SILKIT_BUILD_DOCS=ON`
- Many demos and samples expect `sil-kit-registry`
- Coordinated simulation examples also expect `sil-kit-system-controller`

## Guardrails

- Ignore `_build/`, `_install/`, and `.vs/` as generated or local state
- Avoid editing `ThirdParty/` unless the task is explicitly about dependencies
- This repo expects submodules: `git submodule update --init --recursive`
- External pull requests are currently not accepted; see `CONTRIBUTING.md`

## Good Entry Points

- Build behavior: `CMakeLists.txt`, `CMakePresets.json`, `.github/workflows/build-win.yml`
- Library structure: `SilKit/CMakeLists.txt`, `SilKit/source/CMakeLists.txt`
- Examples: `Demos/`, `docs/code-samples/`

## Editing Heuristics

- Prefer small, localized changes
- If public API changes, inspect both `SilKit/include/` and matching code in `SilKit/source/`
- If CMake changes, inspect both root and module-local `CMakeLists.txt`
- If tests change, consider both unit and integration coverage
- Keep demos and docs aligned with actual runtime expectations


## Code Style
- Files under `SilKit/include/` are public API and should keep the nested `SilKit::...` namespaces.
- Files under `SilKit/source/` are internal implementation and should use the flat `VSilKit::...` namespace.
- Some existing internal code does not follow this everywhere yet; when touching code in `SilKit/source/`, prefer moving new or refactored implementation toward `VSilKit` without adding unrelated churn.
- Use modern C++17 for now.
- Write C++17 in a way that keeps a future migration to C++20 straightforward.
- Prefer composition over inheritance.
- Prefer templates and explicit composition for reusable internal logic and test seams.
- Prefer `std::unique_ptr` and `std::shared_ptr` for owned dynamic memory.
- Prefer the C++ standard library wherever possible over custom utilities.
- Prefer simple, explicit designs over framework-like abstractions or heavy metaprogramming.