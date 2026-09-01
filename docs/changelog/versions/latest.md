# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior

## Fixed

- The SIL Kit Registry could hang on shutdown when a configured dashboard server accepted the
  connection but never answered. Dashboard requests now have connect, write and read deadlines, and
  an in-flight request is aborted after a grace period so that shutdown is always bounded.
- A malformed or unexpected response from the dashboard service no longer terminates the registry's
  dashboard worker thread; unknown fields in the response are now ignored.
- The SIL Kit Registry could call into a destroyed dashboard instance while still serving traffic,
  because the dashboard was torn down before the registry that holds a pointer to it.
- If the dashboard service did not support the bulk-update endpoint, the registry's dashboard worker
  thread stopped while events kept accumulating in an unbounded queue. The capability probe has been
  removed; the worker now keeps draining the queue and reports the failing requests instead. This
  also removes one request during registry startup.
- Dashboard events accumulated but not yet sent are now flushed when the registry shuts down,
  instead of being discarded.
- Log messages from the dashboard instance and its worker thread carry the `Dashboard` log topic
  again; they were previously emitted without a topic and so escaped topic filtering.
- Fix ITest_AsyncSimTask (test failed when run repeatedly)
- Fix the `TimeSyncService` warning about an exceeded soft time limit, which showed a literal `{}` instead of the
  measured timeout in milliseconds

## Changed

- `third-party`: the dashboard client no longer depends on `oatpp`, and the `ThirdParty/oatpp`
  submodule has been removed. The dashboard payloads are now built with the already-bundled
  `rapidyaml`, and the REST requests are issued over the already-bundled standalone `asio`.
  The requests the dashboard service receives are unchanged apart from three cosmetic differences in
  the JSON encoding: a space follows each `:` separator, forward slashes are no longer escaped as
  `\/`, and non-ASCII characters are sent as UTF-8 rather than `\uXXXX` escapes. Control characters
  that cannot be escaped are replaced with U+FFFD.
- Running a build without dashboard support (`SILKIT_BUILD_DASHBOARD=OFF`) and passing
  `--dashboard-uri` now reports plainly that this build has no dashboard support, instead of printing
  an error about a failed dashboard instance creation.
- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
