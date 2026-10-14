# [5.0.9] - UNRELEASED

## Added

- Experimental: a registry with `Experimental.Management.Enable` hosts a management participant in the well-known
  simulation `.well-known/sil-kit/management`. Clients joining it learn the simulations on the registry, their
  participants and required participants, and the configurations of participants that set
  `Experimental.Management.ShareConfiguration`. See `silkit/experimental/management/ManagementDatatypes.hpp`, the
  C++ client in `ManagementClient.hpp`, and the C API `SilKit_Experimental_ManagementClient_*` in
  `silkit/capi/Management.h`. The demo `SilKitDemoManagementClient` prints the simulations on a registry.

## Fixed

- Without virtual time synchronization, the first `CallWithTimeout` of an RPC client could deadlock once another
  RPC client had made a call with timeout. Making that first call from a handler invoked on a timeout aborted the
  process.
- An RPC client dereferenced an invalid iterator when the timeout of an already answered `CallWithTimeout` expired,
  and a reply could race with the timeout of the same call.

## Changed

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  It is the single source of truth for the version numbers; `SilKitVersion.cmake` reads them from it.
  `SilKit/ci/bump_version.py` bumps the version and rotates the changelog. See `docs/development/release.md`.
- The build number, git hash and pre-release suffix are build-time settings (`cmake -DSILKIT_BUILD_NUMBER=N`,
  `-DSILKIT_BUILD_GIT_HASH=<hash>`, `-DSILKIT_VERSION_SUFFIX=rc1`). The git hash defaults to `git rev-parse HEAD`.
  The installed `SilKitVersionMacros.h` only carries fallbacks for these three; query them at runtime via
  `SilKit::Version` or the C API instead.
- Switching between `SetSimulationStepHandler` and `SetSimulationStepHandlerAsync` is now rejected once
  `StartLifecycle()` has been called (`SilKit::StateError`, `SilKit_ReturnCode_WRONGSTATE` in the C API). Setting
  another handler of the same kind at runtime, e.g., to change the step size, remains possible.
