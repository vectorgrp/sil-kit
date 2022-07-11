# SIL Kit Demos

This directory contains sample projects that demonstrate how the SIL Kit
API can be used:

* **CAN, LIN, FlexRay, Ethernet, DataMessage, RPC**:
  Write or read participants that are able to connect to SIL Kit and use buses of
  all supported protocols including CAN, LIN, FlexRay, Ethernet, DataMessage and the RPC service.


* **Benchmark**:
  A simple command line tool that allows to measure SIL Kit simulation performance
  with configurable parameters.

The build system is based on cmake.
Supported target platforms and build tools:
* Ubuntu 18.04 (GCC 7 or later)
* Visual Studio 2015 / 2017


## Build Instructions

For the binary release, the cmake build from the 'SilKit-Demos'
directory should work on all supported platforms in a similar way:

> mkdir build

> cd build

> cmake ../

> cmake --build .

The demos will be placed alongside the binaries.

To build the demos as developer from within the SIL Kit source tree and place them alongside
the binaries, build the 'Demos' CMake target from the SIL Kit 'build' directory:

> cmake --build . --target Demos

On CMake version >=3.12 the '--parallel' flag can be used to speed up
compilation.

* **Note:**
  By default, the demo build system assumes that it resides next to a binary
  release of the SILKIT in a directory 'SilKit' which contains a properly
  exported build of the SILKIT. You can override this by providing your own
  SilKit::SilKit target in cmake.

* **Windows: VisualStudio 2015 and VisualStudio 2017**
  It is possible to use Visual Studio directly, without project files or
  solutions, thanks to the built-in CMake support.
  Open this directory in Explorer, right click and select 'open in Visual
  Studio'. This should start CMake implicitly from Visual Studio.
  Please note that using cmake from command line requires specifying a
  '--config' flag when using a Visual Studio generator, e.g.:

  > cmake --build . --config Debug


Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
