# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior

## Fixed

- Fix ITest_AsyncSimTask (test failed when run repeatedly)
- Fix the `TimeSyncService` warning about an exceeded soft time limit, which showed a literal `{}` instead of the
  measured timeout in milliseconds

## Changed

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  The new `sil-kit-generate-version` maintainer tool regenerates it and performs a complete version bump
  (`SilKitVersion.cmake`, the generated header and the changelog) in one step. See `docs/development/release.md`.
- `SILKIT_BUILD_NUMBER` is now purely a build-time setting (`cmake -DSILKIT_BUILD_NUMBER=N`) rather than a value
  stored in the source tree. The generated header only carries an `#ifndef` fallback of `0`, so
  `SilKit_Version_BuildNumber()` reports `0` unless a build sets it.
- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
