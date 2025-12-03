# [5.0.2] - UNRELEASED

## Fixed

- `asio`: replaced the deprecated (and now removed) methods `asio::io_context::post` and `asio::io_context::dispatch`
  with the suggested alternatives `asio::post` and `asio::dispatch`

- `capi`: fixed an include cycle between `silkit/capi/EventProducer.h` and `silkit/capi/NetworkSimulator.h`
- `sil-kit-monitor`: Fixed missing output due to changes to the default loglevels

## Changed

- Improved the behavior of the `sil-kit-system-controller`, allowing single participants to drop out and rejoin before
  all required participants are connected without having to restart other required participants or the system controller.
- `docs`: document the ability to override the history length of ``DataPublisher`` controllers in the participant configuration

## Added

- `sil-kit-monitor`: Add `-l/--loglevel` commandline arguments to control the log level
