================================
The Vector Integration Bus
================================

The Vector Integration Bus is a runtime component that enables distributed
simulation for automative applications. This README is intended to provide you
with quick start on how to build the Integration Bus.

For documentation on using the Integration Bus, see the html documentation,
which can be generated when building the Integration Bus (cf. Customizing the
Build) and is provided in pre-built form with the Integration Bus packages.


Getting Started - GIT Clone
----------------------------------------

This section specifies the necessary steps to build the integration bus if you
have just cloned the repository.


1. Fetch Third Party Software
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first thing that you should do is initializing the submodules to fetch the
required third party software::

    git submodule update --init --recursive


2. Generate a Project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Integration Bus uses CMake for its build system. CMake can generate a
platform specific project, e.g., a Visual Studio solution or Linux make
files. To generate a project using the default project generator, create a build
directory and configure cmake::

    mkdir build
    cd build
    cmake ..


3. Customize the Build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is often helpful to specify a directory, where the build should be
installed. With cmake, this can be configured via the variable
CMAKE_INSTALL_PREFIX. E.g., to installed the integration bus into a folder
called "install" next to the build folder, run cmake as follows::

    cmake -DCMAKE_INSTALL_PREFIX=../install ..

There are also two IB specific options:

    1. IB_BUILD_DOCS=ON (default: OFF) generates html documentation using
       Doxygen and Sphinx. Both must be installed beforehand. Sphinx is a Python
       program that can be installed via pip.

    2. IB_BUILD_TESTS=OFF (default: ON) disables the generation of unit and
       integration tests. The tests are based on the google test framework,
       which is bundled with the Integration Bus.

    3. IB_BUILD_DEMOS=OFF (default: ON) disables the generation of demo
       applications for the Integration Bus.

    4. IB_BUILD_UTILITIES=OFF (default: ON) disables the generation of utility tools
       (registry, system controller and system monitor).

E.g., if you want to build the Integration Bus with documentation enabled,
call cmake in your build directory as follows::
       
    cmake -DIB_BUILD_DOCS=ON ..

4. Build the Integration Bus
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once the project has been generated, you can build the Integration Bus using the
project specific tools, e.g., by opening the generated Visual Studio or by
running gnu make. You can also start the build process using CMake in a platform
independent way::

    cmake --build .

To install the Integration Bus to a previously configured location, run::

    cmake --build . --target install
