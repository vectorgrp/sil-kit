# [5.0.9] - UNRELEASED

## Changed

- LIN: A slave answers the diagnostic slave response (ID 0x3D) only if a response is pending, and only once,
  as specified by the LIN protocol. `SetFrameResponse` and `UpdateTxBuffer` make a response pending, `Init` does not.
  A header without a pending response is reported as `LIN_RX_NO_RESPONSE`. Before, a slave configured with
  `TxUnconditional` on 0x3D answered every header with its last data. Other LIN IDs, LIN masters and dynamic
  response mode are not affected.

- `SilKitVersionMacros.h` is now committed to the source tree instead of being generated at CMake configure time.
  It is the single source of truth for the version numbers; `SilKitVersion.cmake` reads them from it.
  `SilKit/ci/bump_version.py` bumps the version and rotates the changelog. See `docs/development/release.md`.
- The build number, git hash and pre-release suffix are build-time settings (`cmake -DSILKIT_BUILD_NUMBER=N`,
  `-DSILKIT_BUILD_GIT_HASH=<hash>`, `-DSILKIT_VERSION_SUFFIX=rc1`). The git hash defaults to `git rev-parse HEAD`.
  The installed `SilKitVersionMacros.h` only carries fallbacks for these three; query them at runtime via
  `SilKit::Version` or the C API instead.
