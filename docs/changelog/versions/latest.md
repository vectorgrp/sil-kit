# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior
- New CMake option `SILKIT_BUILD_GENERATE_VERSION` (default `ON`) to build the `sil-kit-generate-version` maintainer
  tool. Turn it off when cross-compiling: the tool runs on the maintainer's machine, so building it for the target
  architecture produces an unrunnable binary.

## Fixed

- Fix ITest_AsyncSimTask (test failed when run repeatedly)
- Fix the `TimeSyncService` warning about an exceeded soft time limit, which showed a literal `{}` instead of the
  measured timeout in milliseconds

## Changed

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  The new `sil-kit-generate-version` maintainer tool regenerates it and performs a complete version bump
  (`SilKitVersion.cmake`, the generated header and the changelog) in one step. See `docs/development/release.md`.
- The build number and git hash are now build-time settings (`cmake -DSILKIT_BUILD_NUMBER=N`,
  `-DSILKIT_BUILD_GIT_HASH=<hash>`) rather than values stored in the source tree. The generated header carries only
  `#ifndef` fallbacks, so a build that passes its own hash makes `SilKit::Version::GitHash()` report the commit
  actually built instead of the commit the header was generated at.
- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
