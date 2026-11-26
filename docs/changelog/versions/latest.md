# [5.0.9] - UNRELEASED

## Added

- Central time coordinator for virtual time synchronization: set `Experimental.TimeSynchronization.TimeCoordinator`
  to `true` on one or more synchronized participants. Exactly one of them is chosen as time coordinator: the one with
  the smallest name among the participants with a coordinated lifecycle. All synchronized participants then report
  their next simulation step to the time coordinator only, which grants the time advances. This reduces the
  synchronization traffic from O(N²) to O(N) messages per simulation step. The API is unchanged. Participants announce
  support with the new `time-coordination` capability; an older participant in a simulation with a time coordinator
  aborts the simulation.

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
