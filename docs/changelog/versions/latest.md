# [5.0.9] - UNRELEASED

## Changed

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  It is the single source of truth for the version numbers; `SilKitVersion.cmake` reads them from it.
  `SilKit/ci/bump_version.py` bumps the version and rotates the changelog. See `docs/development/release.md`.
- The build number, git hash and pre-release suffix are build-time settings (`cmake -DSILKIT_BUILD_NUMBER=N`,
  `-DSILKIT_BUILD_GIT_HASH=<hash>`, `-DSILKIT_VERSION_SUFFIX=rc1`). The git hash defaults to `git rev-parse HEAD`.
  The installed `SilKitVersionMacros.h` only carries fallbacks for these three; query them at runtime via
  `SilKit::Version` or the C API instead.
- Switching between `SetSimulationStepHandler` and `SetSimulationStepHandlerAsync` is now rejected once the
  participant reached the `Running` state (`SilKit::StateError`, `SilKit_ReturnCode_WRONGSTATE` in the C API).
  Setting another handler of the same kind at runtime, e.g., to change the step size, remains possible.
