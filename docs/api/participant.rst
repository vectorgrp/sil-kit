=============
Participant
=============

.. contents:: :local:
   :depth: 1

This document describes the main entry point to the VIB simulation, the participant.
By creating a participant with a given configuration, a connection 
to a simulation is established and the configured participant joins the simulation.

.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>` 

Creating the Participant
~~~~~~~~~~~~~~~~~~~~~~~~
To create an |IParticipant| you have to include the 
:ref:`ib/IntegrationBus.hpp<sec:header-vib-main>` and call the Participant API
factory function::

    auto config = ib::cfg::ParticipantConfigurationFromFile("your_config.yaml");
    auto participant = ib::CreateParticipant(config, "ParticipantName", registryUri);

To take part in the simulation, the Participant needs to be initialized with a proper
configuration, a participant name and optionally the URI of the registry.

.. _sec:iparticipant-api:

The IParticipant API
~~~~~~~~~~~~~~~~~~~~

The instantiated |IParticipant| can then be used to access the other services
of the VIB.

.. admonition:: Warning.

    Services must NOT be created in callbacks. E.g., it is an error to call
    CreateCanController() in the registered callbacks for
    :cpp:func:`CommunicationReadyHandler<ib::mw::sync::ILifecycleService::CommunicationReadyHandler()>`
    or even
    :cpp:func:`SimTask<ib::mw::sync::ITimeSyncService::SetSimulationTask()>`.


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

    .. doxygenfunction:: ib::version::VersionSuffix()

    .. doxygenfunction:: ib::version::GitHash()
