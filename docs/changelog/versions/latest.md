# [5.0.2] - UNRELEASED

## Fixed

- `asio`: replaced the deprecated (and now removed) methods `asio::io_context::post` and `asio::io_context::dispatch`
  with the suggested alternatives `asio::post` and `asio::dispatch`

## Changed

- Improved the behavior of the `sil-kit-system-controller`, allowing single participants to drop out and rejoin before
  all required participants are connected without having to restart other required participants or the system controller.