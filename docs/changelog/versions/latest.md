# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior
- SIL Kit now provides a Software Bill of Materials (SBOM) as an SPDX 2.3 document, `SilKit.spdx.json`.
  It covers a full release and lists the version, license, supplier and package URL of every third
  party component, including those bundled inside another one. For each component it records which
  part of the release it reaches: the SIL Kit library, the `sil-kit-registry` utility, the HTML
  documentation, or the source distribution. Builds also write an SBOM matching their own
  configuration to `<build dir>/sbom/`.

## Fixed

- Fix ITest_AsyncSimTask (test failed when run repeatedly)

## Changed

- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
