VIB-Registry Library
~~~~~~~~~~~~~~~~~~~~

The VIB Registry, required to run the VAsio middleware, can be used as an
extension provided by a non-redistributable shared library.
Normally it is used as a standalone executable for deployments, refer to
:ref:`sec:util-registry` for details.


.. admonition:: Note
    
    Please refer to the ``IntegrationBus-NonRedistributable/README.txt`` in the VIB
    package for redistribution notices.
    Initially, the required vib-registry shared library resides in the 
    ``IntegrationBus-NonRedistributable/`` subdirectory of the official VIB
    package.

Usage
-----
The prerequisites to run the VIB Registry from the shared library are as
follows:

- ensure that the VIB library version and the vib-registry version match.
- the vib-library for your platform and cmake build type should be copied to
  a location specified in the extension search path (see :doc:`../configuration/extension-configuration`).
  On Windows you'll need the vib-registry(d).dll and on Linux
  libvib-registry(d).so

Use the public API described below in your VIB application to implicitly load
and use the shared library.

VIB-Registry API
----------------
The vib-registry shared library needs to reside in the search path specified in the
extension configuration (see :doc:`../configuration/extension-configuration`).
The shared library is loaded, validated and an instance of
:cpp:class:`IIbRegistry<ib::extensions::IIbRegistry>` is constructed.
The corresponding header file is ``ib/extensions/CreateExtension.hpp``.

    .. doxygenfunction:: ib::extensions::CreateIbRegistry
    .. doxygenclass:: ib::extensions::IIbRegistry
       :members:
