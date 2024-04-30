================================
The Vector SIL Kit
================================
.. |ReleaseBadge| image:: https://img.shields.io/github/v/release/vectorgrp/sil-kit.svg
   :target: https://github.com/vectorgrp/sil-kit/releases
.. |LicenseBadge| image:: https://img.shields.io/badge/license-MIT-blue.svg
   :target: https://github.com/vectorgrp/sil-kit/blob/main/LICENSE
.. |DocsBadge| image:: https://img.shields.io/badge/documentation-html-blue.svg
   :target: https://vectorgrp.github.io/sil-kit-docs

.. |AsanBadge| image:: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-asan.yml/badge.svg
   :target: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-asan.yml
.. |UbsanBadge| image:: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-ubsan.yml/badge.svg
   :target: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-ubsan.yml
.. |TsanBadge| image:: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-tsan.yml/badge.svg
   :target: https://github.com/vectorgrp/sil-kit/actions/workflows/linux-tsan.yml
.. |WinBadge| image:: https://github.com/vectorgrp/sil-kit/actions/workflows/build-win.yml/badge.svg
   :target: https://github.com/vectorgrp/sil-kit/actions/workflows/build-win.yml
| |ReleaseBadge| |LicenseBadge| |DocsBadge| 
| |AsanBadge| |UbsanBadge| |TsanBadge| |WinBadge| 

The Vector SIL Kit is an open-source library for connecting Software-in-the-Loop Environments.
This README is intended to provide you with quick start on how to build the Vector SIL Kit.

For documentation on using the Vector SIL Kit, see the HTML documentation,
which can be generated when building the Vector SIL Kit (cf. Customizing the
Build) and is provided in pre-built form with the SIL Kit packages.

The SIL Kit source and documentation is licensed under a permissible open
source license, see LICENSE file. For licenses of third party dependencies,
see `ThirdParty/LICENSES.rst`.

Related Projects
----------------

One of the design goals of SIL Kit is to easily connect different third-party tools,
such as emulators, virtual machines and simulation tools.

The SIL Kit ecosystem comprises the following turn-key solutions:

* The `SIL Kit Adapters for QEMU <https://github.com/vectorgrp/sil-kit-adapters-qemu>`_
  integrates with QEMU to support co-simulation with emulated targets.

* The `SIL Kit Adapters for TAP devices <https://github.com/vectorgrp/sil-kit-adapters-tap>`_
  project provides first-class support for TAP devices of the host operating system.

* The `SIL Kit Adapter for SocketCAN <https://github.com/vectorgrp/sil-kit-adapters-vcan>`_
  can be used to attach a virtual CAN (Controller Area Network) interface (SocketCAN) to a Vector SIL Kit CAN bus.

* The `SIL Kit FMU Importer <https://github.com/vectorgrp/sil-kit-fmu-importer>`_
  allows to import Functional Mockup Units (FMUs) as SIL Kit participants.

Related Applications
--------------------

The SIL Kit ecosystem also offers the following applications:

* The `SIL Kit Dashboard <https://www.vector.com/SIL-Kit-Dashboard/>`_  collects, persists 
  and displays information from different SIL Kit systems.

Getting Started - GIT Clone
----------------------------------------

This section specifies the necessary steps to build the SIL Kit if you
have just cloned the repository.


1. Fetch Third Party Software
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first thing that you should do is initializing the submodules to fetch the
required third party software::

    git submodule update --init --recursive


2. Generate a Project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SIL Kit uses CMake for its build system. CMake can generate a
platform specific project, e.g., a Visual Studio solution or Linux make
files. To generate a project using the default project generator, create a build
directory and configure CMake::

    mkdir build
    cd build
    cmake ..


3. Build the Vector SIL Kit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the project has been generated, you can build the SIL Kit using the
project specific tools, e.g., by opening the generated Visual Studio or by
running gnu make. You can also start the build process using CMake in a platform
independent way::

    cmake --build .

To install the SIL Kit to a previously configured location, run::

    cmake --build . --target install

4. Customize the Build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is often helpful to specify a directory where the build should be
installed. With CMake, this can be configured via the variable
CMAKE_INSTALL_PREFIX. E.g., to install the SIL Kit into a folder
called "install" next to the build folder, run CMake as follows::

    cmake -DCMAKE_INSTALL_PREFIX=../install ..

There are also specific options to toggle details of the build:

    1. SILKIT_BUILD_DOCS=ON (default: OFF) generates HTML documentation using
       Doxygen and Sphinx. Both must be installed beforehand. To install the needed
       dependencies use `pip`:
       `pip3 install -r SilKit/ci/docker/docs_requirements.txt`

    2. SILKIT_BUILD_TESTS=OFF (default: ON) disables the generation of unit and
       integration tests. The tests are based on the GoogleTest framework,
       which is bundled with the SIL Kit.

    3. SILKIT_BUILD_DEMOS=OFF (default: ON) disables the generation of demo
       applications for the SIL Kit.

    4. SILKIT_BUILD_UTILITIES=OFF (default: ON) disables the generation of utility tools
       (sil-kit-registry, sil-kit-system-controller and sil-kit-monitor).

For example, if you want to build the SIL Kit with documentation enabled,
call CMake in your build directory as follows::

    pip3 install -r SilKit/ci/docker/docs_requirements.txt
    pip3 install pipenv
    cmake -D SILKIT_BUILD_DOCS=ON -B _build
    cmake --build _build --target Doxygen

