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
  release of the SIL Kit in a directory 'SilKit' which contains a properly
  exported build of the SIL Kit. You can override this by providing your own
  SilKit::SilKit target in cmake.

* **Windows: VisualStudio 2015 and VisualStudio 2017**
  It is possible to use Visual Studio directly, without project files or
  solutions, thanks to the built-in CMake support.
  Open this directory in Explorer, right click and select 'open in Visual
  Studio'. This should start CMake implicitly from Visual Studio.
  Please note that using cmake from command line requires specifying a
  '--config' flag when using a Visual Studio generator, e.g.:

  > cmake --build . --config Debug


Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
