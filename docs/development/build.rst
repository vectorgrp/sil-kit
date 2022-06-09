======================================
!!! Getting Started as a VIB Developer
======================================

.. contents::


!!! Build Prerequisites
~~~~~~~~~~~~~~~~~~~~~~~
To get started developing you'll need the following prerequisites:
 - `Git`_
 - C++ compiler (Visual Studio 2015/2017, GCC, Clang)
 - `CMake <https://cmake.org>`_
 - `Googletest`_ (submodule)
 - `Json11`_  (submodule)


For generating the documentation, you will require:
 - Python3
 - Sphinx  (e.g. install with *pip install sphinx* version >= 3.0)
 - doxygen


!!! Build Configuration
~~~~~~~~~~~~~~~~~~~~~~~
The VIB build system is based on CMake and can be customized at configuration time.
The following options are available:

.. list-table:: CMake Options

 * - IB_BUILD_TESTS
   - Build the test cases
 * - IB_BUILD_UTILITIES
   - Build the utility tools like the system controller or system monitor.
 * - IB_BUILD_DEMOS
   - Build the demo applications
 * - IB_BUILD_DOCS
   - Build the documentation using doxygen and sphinx
 * - IB_INSTALL_SOURCE
   - Installs the source-tree (used for packaging releases). Implies IB_BUILD_DOCS.

In general, the options can be combined and set using the cmake-gui, your IDE, or command line::

    cmake .. -D IB_BUILD_TESTS=ON -D IB_BUILD_DOCS=ON 


!!! Building Documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~

You will need to set the IB_BUILD_DOCS option::
    
    cmake $source_dir -D IB_BUILD_DOCS=ON

The documentation target is called *Doxygen*::

    cmake --build . --target Doxygen 

Refer to :doc:`rst-help` for guidelines on formatting the documentation.


!!! Packaging
~~~~~~~~~~~~~
VIB uses CPack to generate the release distributions in ZIP form.
It can be packaged using the *package* target::
    
    cmake --build . --target package

The generated package adheres to the following template 
``IntegrationBus-<VERSION>-<compiler>-<OS>-<TYPE>.zip``.
Its contents are as follows:

.. list-table:: Package Contents
   :widths: 25 10 65
   :header-rows: 1
   
   * - Zip Path
     - CMake Option
     - Description
   * - CHANGELOG.rst
     - 
     - Documented changes
   * - IntegrationBus/
     - 
     - The integration bus binaries and cmake config export
   * - IntegrationBus-Demos/
     - IB_INSTALL_SOURCE
     - Source code of the demos. builds against ../IntegrationBus
   * - IntegrationBus-Source/
     - IB_INSTALL_SOURCE
     - The integration bus sources.
   * - IntegrationBus-Documentation/
     - IB_BUILD_DOCS
     - Html documentation


!!! Building the Demos
~~~~~~~~~~~~~~~~~~~~~~

Building the demos from within the source tree is straight forward: 
just build the  ``Demos`` CMake target.
The individual demos are build as a dependency.

The distributed Demos, as packaged by CPack, are preconfigured to build against 
a copy of the VIB binaries in ``../IntegrationBus/`` .
This can be overriden by providing your own ``IntegrationBus`` CMake target library,
before the demos are configured by cmake.
Or by changing the ``find_package(IntegrationBus ... PATHS path/to/VIB)`` statement directly
in the ``IntegrationBus-Demos/CMakeLists.txt`` directory.


!!! Architecture
~~~~~~~~~~~~~~~~

Have a look at our :ref:`architecture overview <base-architecture>`.


.. _CMake: https://cmake.org
.. _Git: https://git-scm.org
.. _Googletest: https://github.com/google/googletest/blob/master/googletest/docs/primer.md
.. _Json11: https://github.com/dropbox/json11

