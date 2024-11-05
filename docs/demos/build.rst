.. include:: /substitutions.rst

==================
Building the Demos
==================

The |ProductName| Demos are not available as pre-built binaries and have to be compiled from source first.

* If you plan to use the demos as a starting point for further development, start by cloning the `SIL Kit git repository <https://github.com/vectorgrp/sil-kit>`_.
  Don't forget to call ``git submodule update --init --recursive`` after cloning the repository for the first time.
  Using the build instructions below will build the complete |ProductName| library including the Demos.
* If you just want to try out the demos, download a `SIL Kit release package <https://github.com/vectorgrp/sil-kit/releases>`_ which also includes all sources.
  Using the build instructions below will only build the |ProductName| Demos and use the precompiled library of the package.
* Building the Demos from the installation folder of the SIL Kit MSI Installer is strongly discouraged, as all involved tools require administrative privileges if the default installation directory is used.

There are several options to build the demos, all require the installation of a C++ compiler and CMake.

.. admonition:: Note

    If you don't have any experience with setting up C++ / CMake projects, we recommend using VS Code and a `SIL Kit release package <https://github.com/vectorgrp/sil-kit/releases>`_.
    VS Code is free to use, and the prerequisites can be quickly set up using the recommended VS Code extensions.

VS Code
-------

#. Install the VS Code extensions ``C/C++`` and ``CMake`` 
#. Open the folder with VS Code

   a. For the git repository: open the root folder of the repository
   b. For a |ProductName| package: open the ``SilKit-Demos`` folder
#. Opening the folder automatically starts the CMake configuration step.
   You can also manually call this step in the CMake extension page under ``Project Status | Configure``.
#. In the CMake extension page, build the project under ``Project Status | Build``
#. Locate the binaries

   a. For the git repository: The binaries reside in ``_build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

Visual Studio
-------------

#. Make sure Visual Studio is set up for C++ and CMake is installed
#. Open the folder with Visual Studio

   a. For the git repository: open the root folder of the repository
   b. For a |ProductName| package: open the ``SilKit-Demos`` folder
#. Opening the folder automatically starts the CMake configuration step. 
   You can also manually call this step under ``Project | Configure Cache``.
#. Build the project with ``Build | Build All``
#. Locate the binaries

   a. For the git repository: The binaries reside in ``_build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

From command line
-----------------
 
#. Make sure a C++ compiler and CMake is installed
#. Open a terminal

   a. For the git repository: navigate to the root folder of the repository
   b. For a |ProductName| package: navigate to ``<package folder>/SilKit-Demos``
#. Run the following commands:
   
   * ``mkdir build``
   * ``cd build``
   * ``cmake ..``
   * ``cmake --build .``
#. Locate the binaries

   a. For the git repository: The binaries reside in ``build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

Due to the large variety of terminals, different operating systems and availability of environment variables, the previous instruction can fail for several reasons.
Usually, the configuration step performed in  ``cmake ..`` will give useful hints of what went wrong.

Advanced CMake configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Build the Demos from a |ProductName| package against your own target library
   The distributed Demos, as packaged by CPack, are preconfigured to build against a copy of the |ProductName| binaries in ``../SilKit/``.
   This can be overridden by providing your own ``SilKit`` CMake target library, before the demos are configured by CMake.
   Or by changing the ``find_package(SilKit ... PATHS path/to/SilKit)`` statement directly in the ``./SilKit-Demos/CMakeLists.txt`` directory.
