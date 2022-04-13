=============
Participant
=============

.. contents:: :local:
   :depth: 1

This document described the main entry point to the VIB simulation.
The Participant layer provides the interface between a middleware, which implements
the distributed communication, and the high level simulation services.
Refer to :ref:`Base Architecture<base-architecture>` for an overview of the VIB.

.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>` 

Accessing the Participant
~~~~~~~~~~~~~~~~~~~~~~~~
To create an |IParticipant| you have to include the 
:ref:`ib/IntegrationBus.hpp<sec:header-vib-main>` and call the :ref:`Participant API<sec:participant-factory>`
factory function::

    auto config = ib::cfg::Config::FromJsonFile("your_config.json");
    auto participant = ib::CreateParticipant(config, "ParticipantName", domainId);

To take part in the simulation, the Participant needs to be initialized with a proper
configuration, a participant name and a domain ID.

.. _sec:iparticipant-api:

The IParticipant API
~~~~~~~~~~~~~~~~~~~

The instantiated |IParticipant| can then be used to access the other services
of the VIB.

.. admonition:: Warning.

    Services must NOT be created in callbacks. E.g., it is an error to call
    CreateCanController() in the registered callbacks for
    :cpp:func:`InitHandler<ib::mw::sync::IParticipantController::SetInitHandler()>`
    or even
    :cpp:func:`SimTask<ib::mw::sync::IParticipantController::SetSimulationTask()>`.


.. doxygenclass:: ib::mw::IParticipant
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
..
..    .. doxygenstruct:: ib::mw::EndpointAddress
..       :members:
..
