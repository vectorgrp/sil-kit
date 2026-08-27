# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior
- The documentation now lists every third party dependency found in the source tree with its
  version and license on the Licenses page, followed by the full license text of each component
  that ships one.

## Fixed

- Fix ITest_AsyncSimTask (test failed when run repeatedly)
- The third party license notices were incomplete. They now also cover components that are vendored
  inside another dependency, such as c4core inside the rapidyaml sources, and the components that
  ship inside the HTML documentation. `ThirdParty/LICENSES.rst` is generated from a software bill of
  materials and is the file the documentation shows, so the two can no longer disagree.

## Changed

- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
