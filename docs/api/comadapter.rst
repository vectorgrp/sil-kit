=============
ComAdapter
=============

.. contents:: :local:
   :depth: 1

This document described the main entry point to the VIB simulation.
The ComAdapter layer provides the interface between a middleware, which implements
the distributed communication, and the high level simulation services.
Refer to :ref:`Base Architecture<base-architecture>` for an overview of the VIB.

.. |IComAdapter| replace:: :cpp:class:`IComAdapter<ib::mw::IComAdapter>` 

Accessing the ComAdapter
~~~~~~~~~~~~~~~~~~~~~~~~
To create an |IComAdapter| you have to include the 
:ref:`ib/IntegrationBus.hpp<sec:header-vib-main>` and call the :ref:`ComAdapter API<sec:comadapter-factory>`
factory function::

    auto config = ib::cfg::Config::FromJsonFile("your_config.json");
    auto comAdapter = ib::CreateComAdapter(config, "ParticipantName", domainId);

To take part in the simulation, the ComAdapter needs to be initialized with a proper
configuration, a participant name and a domain ID.

.. _sec:icomadapter-api:

The IComAdapter API
~~~~~~~~~~~~~~~~~~~
The instantiated |IComAdapter| can then be used to access the other services
of the VIB.

    .. doxygenclass:: ib::mw::IComAdapter
       :members:


VIB Version
~~~~~~~~~~~
Version information about the currently running VIB instance
can be queried using the following functions:

    .. doxygenfunction:: ib::version::Major()

    .. doxygenfunction:: ib::version::Minor()

    .. doxygenfunction:: ib::version::Patch()

    .. doxygenfunction:: ib::version::String()

    .. doxygenfunction:: ib::version::BuildNumber()

    .. doxygenfunction:: ib::version::SprintNumber()

    .. doxygenfunction:: ib::version::SprintName()

    .. doxygenfunction:: ib::version::GitHash()

..          
..    .. doxygenfunction:: ib::CreateFastRtpsComAdapter
..
..    .. doxygenstruct:: ib::mw::EndpointAddress
..       :members:
..
