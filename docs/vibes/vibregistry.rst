VIB-Registry
~~~~~~~~~~~~

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

VIB-Registry API
----------------
The vib-registry shared library needs to reside in the process's current working
directory for this function to work properly.
The shared library is loaded, validated and an instance of
:cpp:class:`IIbRegistry<ib::extensions::IIbRegistry>` is constructed.

    .. doxygenfunction:: ib::extensions::CreateIbRegistry
    .. doxygenclass:: ib::extensions::IIbRegistry
       :members:
