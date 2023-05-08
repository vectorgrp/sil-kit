=============
Participant
=============

.. contents:: :local:
   :depth: 1

This document describes the main entry point to the SIL Kit simulation, the participant.
By creating a participant with a given configuration, a connection 
to a simulation is established, and the configured participant joins the simulation.

.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>` 

Creating the Participant
~~~~~~~~~~~~~~~~~~~~~~~~
To create an |IParticipant| you have to include the 
:ref:`silkit/SilKit.hpp<sec:header>` and call the Participant API
factory function::

    auto config = SilKit::Config::ParticipantConfigurationFromFile("my_config.yaml");
    auto participant = SilKit::CreateParticipant(config, "ParticipantName", registryUri);

To take part in the simulation, the participant needs to be initialized with a proper
configuration, a participant name and optionally the URI of the registry:

.. doxygenfunction:: SilKit::CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig, const std::string &participantName) -> std::unique_ptr<IParticipant>
.. doxygenfunction:: SilKit::CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig, const std::string &participantName, const std::string &registryUri) -> std::unique_ptr<IParticipant>

.. _sec:iparticipant-api:

IParticipant API
~~~~~~~~~~~~~~~~

The instantiated |IParticipant| can then be used to create and access services of the SIL Kit.
A controller name (the ``canonicalName`` given in Create*-Calls) must be unique within a controller type, using the same name twice results in
a ``ConfigurationError``.

.. admonition:: Warning.

    Services must NOT be created in callbacks. E.g., it is an error to call ``CreateCanController()`` in a 
    ``CommunicationReadyHandler`` or even ``SimulationStepHandler``.

.. doxygenclass:: SilKit::IParticipant
   :members:


SIL Kit Version
~~~~~~~~~~~~~~~

Version information about the currently running SIL Kit instance
can be queried using the following functions:

    .. doxygenfunction:: SilKit::Version::Major()

    .. doxygenfunction:: SilKit::Version::Minor()

    .. doxygenfunction:: SilKit::Version::Patch()

    .. doxygenfunction:: SilKit::Version::String()

    .. doxygenfunction:: SilKit::Version::BuildNumber()

    .. doxygenfunction:: SilKit::Version::VersionSuffix()

    .. doxygenfunction:: SilKit::Version::GitHash()
