# [5.0.2] - 2025-12-04

## Fixed

- `asio`: replaced the deprecated (and now removed) methods `asio::io_context::post` and `asio::io_context::dispatch`
  with the suggested alternatives `asio::post` and `asio::dispatch`

- `capi`: fixed an include cycle between `silkit/capi/EventProducer.h` and `silkit/capi/NetworkSimulator.h`

- `sil-kit-monitor`: fixed missing output due to changes to the default loglevels

- `logs`: fixed logging in JSON format without any key-value pairs

- `docs`: fixed broken formatting in `troubleshooting/advanced`

- `ci`: fixed usage of cmake for macOS runners

- `cmake`: fixed QNX preset and toolset

- `git`: fixed broken build dir glob pattern in `.gitignore` file

- `quality`: fixed various warnings

## Changed

- `sil-kit-system-controller`: improved the behavior of the `sil-kit-system-controller`, allowing single participants
  to drop out and rejoin before all required participants are connected without having to restart other required
  participants or the system controller.

- `docs`: document the ability to override the history length of `DataPublisher` controllers in the participant
  configuration

- `sil-kit-registry`: enable collecting metrics by default

- `tests`: added timeout for the participant modes test and a separate (overall) timeout for test execution in the CI

- `cmake`: added the `distrib` preset and removed various superfluous presets

- `docs`: document the (experimental) configuration settings that influence metrics generation and collection

- `quality`: made multiple derived classes `final`

## Added

- `sil-kit-monitor`: add `-l` / `--loglevel` commandline arguments to control the log level

- `logs`: added message target in the `To` key-value field of trace messages

- `logs`: add trace logs for sending a historized pub/sub messages

- `logs`: added a `raw` key for arbitrary JSON objects in log messages

- `ci`: added `clang-tidy` to the CI

- `ci`: added devcontainer for `clang-format`, format all files, and ensure that the check is enabled in the CI

- `ci`: added a check to ensure that all commits are properly `Signed-of-by` as defined by
  [DCO](https://wiki.linuxfoundation.org/dco)
