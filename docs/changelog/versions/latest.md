# [5.0.5] - UNRELEASED

## Fixed

- `lin`: fixed missing reception of the Go-To-Sleep frame by the master after `GoToSleep()` in detailed simulation


## Changed

- `cmake`: merged almost all internal CMake `INTERFACE` libraries into `I_SilKit`
- `third-party`: update `oatpp` to version 1.3.1
- `docs`: update the supported platforms table
- `quality`: following small improvements
  - add missing `#pragma once`
  - add `static` to some TU-local functions
  - add `inline` to some header-defined functions
  - remove an obsolete source file
  - adjust warning flags
