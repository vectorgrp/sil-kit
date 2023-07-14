# SIL Kit Demos

This directory contains sample projects that demonstrate how the SIL Kit
API can be used:

* **CAN, LIN, FlexRay, Ethernet, Pub/Sub, RPC**:
  Write or read participants that are able to connect to SIL Kit and use buses of
  all supported protocols including CAN, LIN, FlexRay, Ethernet, Pub/Sub and the RPC service.

The build system is based on cmake.
Supported target platforms and build tools:
* Ubuntu 18.04 (GCC 7 or later)
* Visual Studio 2017

## Build Instructions

For the binary distribution, the cmake build from the 'SilKit-Demos'
directory should work on all supported platforms in a similar way:

> cmake -B build

> cmake --build build

You might want to add the appropriate '-T toolset' and '-A architecture' options on MSVC builds.
The demos will be placed alongside the binaries, if the binary distribution is unpacked next
to the SilKit-Source package.
For example, if the directory layout looks as follows, the demo executables will
be placed in `SilKit-X.Y.Z-$Platform/bin`:

> SilKit-X.Y.Z-Source/Demos

> SilKit-X.Y.Z-$Platform/


You can also specify the location of the unpacked binary package using the `SILKIT_DIR` variable:

> cmake -B build -D SILKIT_DIR=path/to/SilKit-X.Y.Z-PlatformEtc


To build the demos as developer from within the SIL Kit source tree and place them alongside
the binaries, build the 'Demos' CMake target from the SIL Kit 'build' directory:

> cmake --build . --target Demos

On CMake version >=3.12 the '--parallel' flag can be used to speed up
compilation.

* **Note:**
  By default, the demo build system assumes that it resides next to a binary
  release of the SIL Kit in a directory 'SilKit' which contains a properly
  exported build of the SIL Kit. You can override this by providing your own
  SilKit::SilKit target in cmake.

* **Windows: Visual Studio 2017 and later**
  It is possible to use Visual Studio directly, without project files or
  solutions, thanks to the built-in CMake support.
  Open this directory in Explorer, right click and select 'open in Visual
  Studio'. This should start CMake implicitly from Visual Studio.
  Please note that using cmake from command line requires specifying a
  '--config' flag when using a Visual Studio generator, e.g.:

  > cmake --build . --config Debug

