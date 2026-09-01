# [5.0.8] - UNRELEASED


## Added

- Add Integration Test for Timestamp Behavior
- The documentation now lists every third party dependency found in the source tree with its
  version and license on the Licenses page, followed by the full license text of each component
  that ships one.
- New CMake option `SILKIT_BUILD_GENERATE_VERSION` (default `ON`) to build the `sil-kit-generate-version` maintainer
  tool. Turn it off when cross-compiling: the tool runs on the maintainer's machine, so building it for the target
  architecture produces an unrunnable binary.

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
- The third party license notices were incomplete. They now also cover components that are vendored
  inside another dependency, such as c4core inside the rapidyaml sources, and the components that
  ship inside the HTML documentation. `ThirdParty/LICENSES.rst` is generated from a software bill of
  materials and is the file the documentation shows, so the two can no longer disagree.

## Changed

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  The new `sil-kit-generate-version` maintainer tool regenerates it and performs a complete version bump
  (`SilKitVersion.cmake`, the generated header and the changelog) in one step. See `docs/development/release.md`.
- The build number, git hash and pre-release suffix are now build-time settings (`cmake -DSILKIT_BUILD_NUMBER=N`,
  `-DSILKIT_BUILD_GIT_HASH=<hash>`, `-DSILKIT_VERSION_SUFFIX=rc1`) rather than values stored in the source tree.
  The generated header carries only `#ifndef` fallbacks, so a build that passes its own hash makes
  `SilKit::Version::GitHash()` report the commit actually built, and a build that sets a suffix reports
  `5.0.8-rc1` from `SilKit::Version::String()` and in the CPack archive name.
- `third-party`: the dashboard client no longer depends on `oatpp`, and the `ThirdParty/oatpp`
  submodule has been removed. The dashboard payloads are now built with the already-bundled
  `rapidyaml`, and the REST requests are issued over the already-bundled standalone `asio`.
  The requests the dashboard service receives are unchanged apart from three cosmetic differences in
  the JSON encoding: a space follows each `:` separator, forward slashes are no longer escaped as
  `\/`, and non-ASCII characters are sent as UTF-8 rather than `\uXXXX` escapes. Control characters
  that cannot be escaped are replaced with U+FFFD.
- `third-party`: the SIL Kit build now defines `ASIO_NO_DEPRECATED`, so that using an asio interface
  that upstream has deprecated is a build error rather than something that only surfaces when asio
  removes it. No SIL Kit source used one.
- Running a build without dashboard support (`SILKIT_BUILD_DASHBOARD=OFF`) and passing
  `--dashboard-uri` now reports plainly that this build has no dashboard support, instead of printing
  an error about a failed dashboard instance creation.
- A `--dashboard-uri` whose scheme is not `http` is now rejected when the dashboard instance is
  created. The dashboard client only ever spoke plaintext HTTP, so an `https://` URI was previously
  accepted and silently downgraded.
- Changes to the SIL KIT MSI installer: 
  - Default installation path changed from `<ProgramFilesFolder>\Vector SIL Kit <VERSION>` to `<ProgramFilesFolder>\SIL Kit <VERSION>`
  - Windows System Service Name changed from `VectorSilKitRegistry` to `SilKitRegistry`
  - SIL Kit Registry System Service config file installation path changed from `<ProgramDataFolder>\Vector SIL Kit\silkit-registry.yaml` to `<ProgramDataFolder>\SIL Kit\silkit-registry.yaml`
  - Note that the `<ProgramDataFolder>\Vector SIL Kit` is not removed by installing `SilKit-5.0.8.msi`.
- `TimeSyncService` now throws `LogicError{"TimeSyncPolicy is not set"}` if `CompleteSimulationStep` is used before `StartLifecyle`.
