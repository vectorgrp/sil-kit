# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior

## Fixed

- Fix ITest_AsyncSimTask (test failed when run repeatedly)
- Fix the `TimeSyncService` warning about an exceeded soft time limit, which showed a literal `{}` instead of the
  measured timeout in milliseconds

## Changed

- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
