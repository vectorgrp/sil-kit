:orphan:

==========================================
!!! Getting Started as a SIL Kit Developer
==========================================

.. contents::


!!! Build Prerequisites
~~~~~~~~~~~~~~~~~~~~~~~
To get started developing you'll need the following prerequisites:
 - `Git`_
 - C++ compiler (Visual Studio 2017 or newer, GCC, Clang)
 - `CMake <https://cmake.org>`_
 - C++ dependencies are managed as submodules, e.g. run `git submodule update --init --recursive`


For generating the documentation, you will require:
 - Python3 dependencies, see: `SilKit/ci/docker/docs_requirements.txt`
    - install with `pip3 install -r SilKit/ci/docker/docs_requirements.txt`
    - and pipenv: `pip3 install pipenv`
 - doxygen


!!! Build Configuration
~~~~~~~~~~~~~~~~~~~~~~~
The SIL Kit build system is based on CMake and can be customized at configuration time.
The following options are available:

.. list-table:: CMake Options

 * - SILKIT_BUILD_TESTS
   - Build the test cases
 * - SILKIT_BUILD_UTILITIES
   - Build the utility tools like the System Controller or Monitor.
 * - SILKIT_BUILD_DEMOS
   - Build the demo applications
 * - SILKIT_BUILD_DOCS
   - Build the documentation using Doxygen and Sphinx
 * - SILKIT_INSTALL_SOURCE
   - Installs the source-tree (used for packaging releases). Implies SILKIT_BUILD_DOCS.

In general, the options can be combined and set using the CMake GUI, your IDE, or command line::

    cmake .. -D SILKIT_BUILD_TESTS=ON -D SILKIT_BUILD_DOCS=ON 


!!! Building Documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~

Install the required Python dependencies::

    pip3 install -r SilKit/ci/docker/docs_requirements.txt
    pip3 install pipenv
    
You will need to set the SILKIT_BUILD_DOCS option::
    
    cmake $source_dir -D SILKIT_BUILD_DOCS=ON

The documentation target is called *Doxygen*::

    cmake --build . --target Doxygen 

Refer to :doc:`rst-help` for guidelines on formatting the documentation.


!!! Packaging
~~~~~~~~~~~~~
SIL Kit uses CPack to generate the release distributions in ZIP form.
It can be packaged using the *package* target::
    
    cmake --build . --target package

The generated package adheres to the following template 
``SilKit-<VERSION>-<compiler>-<OS>-<TYPE>.zip``.
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
   * - SilKit/
     - 
     - The SIL Kit binaries and CMake config export
   * - SilKit-Demos/
     - SILKIT_INSTALL_SOURCE
     - Source code of the demos, builds against ``../SilKit``
   * - SilKit-Source/
     - SILKIT_INSTALL_SOURCE
     - The SIL Kit sources.
   * - SilKit-Documentation/
     - SILKIT_BUILD_DOCS
     - Html documentation


!!! Building the Demos
~~~~~~~~~~~~~~~~~~~~~~

Building the demos from within the source tree is straight forward: 
just build the ``Demos`` CMake target.
The individual demos are build as a dependency.

The distributed Demos, as packaged by CPack, are preconfigured to build against 
a copy of the SIL Kit binaries in ``../SilKit/``.
This can be overriden by providing your own ``SilKit`` CMake target library,
before the demos are configured by CMake.
Or by changing the ``find_package(SilKit ... PATHS path/to/SilKit)`` statement directly
in the ``SilKit-Demos/CMakeLists.txt`` directory.


!!! Architecture
~~~~~~~~~~~~~~~~

..
   Have a look at our :ref:`architecture overview <base-architecture>`.


.. _CMake: https://cmake.org
.. _Git: https://git-scm.org
.. _Googletest: https://github.com/google/googletest/blob/master/googletest/docs/primer.md
.. _Json11: https://github.com/dropbox/json11

