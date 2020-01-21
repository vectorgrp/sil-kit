====================================
Getting Started as a VIB Developer
====================================

.. contents::

Build Preqrequisites
~~~~~~~~~~~~~~~~~~~~
To get started developing you'll need the following prerequisites:
 - `Git`_
 - C++ compiler (Visual Studio 2015/2017, GCC, Clang)
 - `CMake <https://cmake.org>`_
 - `FastRTPS`_ (submodule, for the FastRTPS middleware layer)
 - `Googletest`_ (submodule)
 - `Json11`_  (submodule)

As a user convenience, the cmake build system will attempt to download a git checkout of FastRTPS from github if it can't find the submodule.

For generating the documentation you'll require:
 - Python3
 - Sphinx  (e.g. install with *pip install sphinx*)
 - doxygen

Build Configuration
~~~~~~~~~~~~~~~~~~~
The VIB build system is based on CMake and can be configured at configration time.
The following options are available:

.. list-table:: CMake Options

 * - IB_BUILD_TESTS
   - build the test cases
 * - IB_BUILD_DOCS
   - build the documentation using doxygen and sphinx
 * - IB_BIN_FASTRTPS_ENABLE
   - use prebuilt FastRTPS binaries
 * - IB_BIN_FASTRTPS_REPOSITORY
   - path to the FastRTPS binary zip file
 * - IB_INSTALL_SOURCE
   - Installs the source-tree (used for packaging releases). Implies IB_BUILD_DOCS.

In general the options can be combined and set using the cmake-gui, your IDE, or command line::

    cmake .. -D IB_BUILD_TESTS=ON -D IB_BUILD_DOCS=ON 



Building Documentation
~~~~~~~~~~~~~~~~~~~~~~

You'll need to set the IB_BUILD_DOCS option::
    
    cmake $source_dir -D IB_BUILD_DOCS=ON

The documentation target is called *Doxygen*::

    cmake --build . --target Doxygen 

Refer to :doc:`rst-help` for guidelines on formatting the documentation.

Packaging
~~~~~~~~~
VIB uses CPack to generate the release distributions in ZIP form.
It can be packaged using the *package* target::
    
    cmake --build . --target package

The generated package adheres to the following template 
``IntegrationBus-<VERSION>-<compiler>-<OS>-<TYPE>.zip``.
Its contents are as follows:

.. list-table:: package contents
   :widths: 25 10 65
   :header-rows: 1
   
   * - Zip Path
     - CMake Option
     - Description
   * - CHANGELOG.rst
     - 
     - documented changes
   * - IntegrationBus/
     - 
     - the integration bus binaries and cmake config export
   * - IntegrationBus-Demos/
     - IB_INSTALL_SOURCE
     - source code of the demos. builds against ../IntegrationBus
   * - IntegrationBus-Source/
     - IB_INSTALL_SOURCE
     - the integration bus sources.
   * - IntegrationBus-Documentation/
     - IB_BUILD_DOCS
     - html documentation


.. _sec:build-demos:

Building the Demos
~~~~~~~~~~~~~~~~~~

Building the demos from within the source tree is straight forward: 
just build the  ``Demos`` CMake target.
The individual demos are build as a dependency.

The distributed Demos, as packaged by CPack, are preconfigured to build against 
a copy of the VIB binaries in ``../IntegrationBus/`` .
This can be overriden by providing your own ``IntegrationBus`` CMake target library,
before the demos are configured by cmake.
Or by changing the ``find_package(IntegrationBus ... PATHS path/to/VIB)`` statement directly
in the ``IntegrationBus-Demos/CMakeLists.txt`` directory.



Architecture
~~~~~~~~~~~~

Have a look at our :ref:`architecture overview <base-architecture>`.


.. _CMake: https://cmake.org
.. _Git: https://git-scm.org
.. _FastRTPS: https://github.com/eProsima/Fast-RTPS
.. _Googletest: https://github.com/google/googletest/blob/master/googletest/docs/primer.md
.. _Json11: https://github.com/dropbox/json11

