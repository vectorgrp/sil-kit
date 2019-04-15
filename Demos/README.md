# IntegrationBus Demos

This folder contains sample projects that demonstrate how the IntegrationBus API can be used to:

* *Can, Lin, FlexRay, Ethernet, IO, GenericMessage*:
  Write participants that are able to connect to IntegrationBus and use buses of all supported protocols,
  including CAN, LIN, FlexRay, Ethernet, I/O (analog, digital, pattern signals and PWM), and a Generic Messages.
* *ExecutionController, ExecutionControllerProxy*:
  Instantiating a participant that serves as a timing master for controlling the simulation execution via a quantum-based approach.
  Instantiating a participant that transmits commands to start/stop the simulation.
* *TickTickDone*:
  Installing a Tick-TickDone-based synchronization mechanism.
* *ConfigBuilder*:
  Processing JSON configuration files that adhere to the IbConfig.schema.json format.

Supported target platforms and build tools:

* *Linux: GCC 4.9 or later*
  Run 'make' from within this folder to execute the provided GNU Makefiles
* *Windows: VisualStudio 2015 and VisualStudio 2017*
  Open IntegrationBus-Demos_vs140.sln
* *Linux and Windows: CMake 3.5 or later*
  * Run 'cmake . -DCMAKE_PREFIX_PATH=..' to determine available build systems and create build scripts for one of them.
    Note:
      The 'CMAKE_PREFIX_PATH' hints CMake to package config files (usually a folder that contains the ./cmake subfolder), 
      so that building against the IntegrationBus library of the matching target architecture is possible.
    To set the build system and target system, you can append further arguments, for example:
    * '-A Win32 -T v141' for 32bit executables and the VisualStudio 2017 compiler under Windows
    * '-A x64 -T v140' for 64bit executables and the VisualStudio 2015 compiler under Windows
  * Run 'cmake --build . --config Debug' to execute the previously generated build scripts for target 'Debug'.
  You can check CMake's man pages or the [online help](https://cmake.org/documentation/) on how to set further options.

Copyright (c) Vector Informatik GmbH. All rights reserved.
