========================================
Configuration
========================================

.. _sec:sil-kit-config-yaml:

The Vector SIL Kit provides optional configuration to enhance flexibility and control through use of YAML files.
This allows users to modify behavior without recompiling source code.
Further, it allows configuration of services which are inactive by default, such as tracing and logging.

.. contents::
   :local:
   :depth: 2

.. _sec:participant-config:

The Participant Configuration File
=======================================

Vector SIL Kit simulation participants are primarily configured by developers in source code using the SIL Kit API.
However, the following scenarios are conceivable in which some flexibility is useful even after compile-time:

- A simulation participant is distributed in binary form to integrators. In order to integrate such 
  participants into simulations, an integrator must be able to post-configure some aspects of the participant to make it fit,
  particularly its properties, behavior and how it interconnects within a simulation.
- A simulation consists of multiple participants that share the same behavior. An integrator would have to ask developers to compile 
  these participants from the same sources multiple times, with only the participant names differing.
- Developers or integrators want to temporarily enable debugging features without the need or the ability to recompile the sources, 
  for example logging, tracing, and health checking.

To cover these scenarios, Vector SIL Kit offers the ability to modify a participant's configuration via participant configuration files,
often simply referred to as *participant configuration*. The feature can be enabled by passing the file as an argument when a participant 
is created, :cpp:func:`CreateParticipant(std::shared_ptr\<SilKit::Config::IParticipantConfiguration\> participantConfig, ...)<SilKit::CreateParticipant()>`.

Participant configuration files allow to override a subset of parameters which are configurable via the SIL Kit API.
Configuration parameters that are specified within the participant configuration override corresponding 
programmatically defined values. For example, the ``ParticipantName`` field of the participant configuration overrides the 
participant name that is provided through the API of the Vector SIL Kit when participants are created, namely 
:cpp:func:`CreateParticipant(..., const std::string& participantName, ...)<SilKit::CreateParticipant()>`. This gives integrators the ability to run
a simulation with multiple instances of a participant from a single implementation.

.. admonition:: Note

    It is recommended that all configuration be done at the source code level so that an empty configuration file can be provided for most simulation runs.
    A user should be able to create (or modify an existing) configuration file for a participant, if behavior or network connections need to be reconfigured.

A participant configuration file is written in YAML syntax according to a specified schema. It begins with some general information 
about the configuration file itself, followed by several subsections for the different services of the Vector SIL Kit.

.. admonition:: Note

    Many IDEs automatically support participant configuration schema support when the participant configuration file ends with the suffix ``.silkit.json/yaml``.

The outline of a participant configuration file is as follows:

.. code-block:: yaml
                
    ---
    "$schema": "./ParticipantConfiguration.schema.json"
    schemaVersion: 1
    Description: Sample configuration with all major subsections
    ParticipantName: Participant1
    CanControllers:
    - ...
    LinControllers: 
    - ...
    FlexrayControllers: 
    - ...
    EthernetControllers: 
    - ...
    DataPublishers: 
    - ...
    DataSubscribers: 
    - ...
    RpcClients: 
    - ...
    RpcServers: 
    - ...
    Logging: 
    - ...
    HealthCheck: 
    - ...
    Tracing:
      ...
    Extensions: 
    - ...
    Middleware: 
    - ...

.. _subsec:participant-config-overview:

Overview
~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - $schema
     - The location of the participant configuration schema file. The ``ParticipantConfiguration.schema.json`` is
       part of the SIL Kit sources and can be found in the folder
       ``./SilKit/source/config/``.

   * - schemaVersion
     - The version of the used participant configuration schema. Current version is 1.
       
   * - ParticipantName
     - The name of the simulation participant that joins the Vector SIL Kit simulation. Overrides a programmatically
       defined participant name.

   * - :doc:`CanControllers<configuration-services>`, 
       :doc:`LinControllers<configuration-services>`,
       :doc:`FlexrayControllers<configuration-services>`,
       :doc:`EthernetControllers<configuration-services>`,
       :doc:`DataPublishers<configuration-services>`,
       :doc:`DataSubscribers<configuration-services>`,
       :doc:`RpcClients<configuration-services>`,
       :doc:`RpcServers<configuration-services>`
     - These sections are used to configure bus controllers and other communication services.

   * - :ref:`Logging<sec:cfg-participant-logging>`
     - The logger configuration for this participant.

   * - :ref:`HealthCheck<sec:cfg-participant-healthcheck>`
     - Configuration concerning soft and hard timeouts for simulation task execution.

   * - :ref:`Tracing<sec:cfg-participant-tracing>`
     - Configuration of experimental tracing and replay functionality.

   * - :doc:`Extensions<extension-configuration>`
     - Configuration of optional extensions to the Vector SIL Kit and where to find them.

   * - :doc:`Middleware<middleware-configuration>`
     - This optional section can be used to configure the middleware running the Vector SIL Kit.
       If this section is omitted, defaults will be used.

.. _subsec:participant-config-options:

Configuration Options
~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2

   configuration-services
   logging-configuration
   healthcheck-configuration
   configuration-tracing
   extension-configuration
   middleware-configuration

.. _sec:registry-config:

The Registry Configuration File
===============================

An instance of the SIL Kit Registry (``sil-kit-registry(.exe)``) can be
configured via a YAML file.
The configuration file is optional and overrides the settings specified on the
command line.
It also allows extended configuration, beyond what the command line allows,
particularly for logging.

The outline of a registry configuration file is as follows:

.. code-block:: yaml

    ---
    SchemaVersion: 1
    Description: Sample registry configuration.

    ListenUri: silkit://localhost:8500

    Logging:
      Sinks:
        - Type: Stdout
          Level: Trace
        - Type: File
          Level: Trace
          LogName: SampleRegistryLogFile
    DashboardUri: http://localhost:8082

.. _subsec:registry-config-overview:

Overview
~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - ``SchemaVersion``
     - The version of the used registry configuration schema. Current version is 1.

   * - ``Description``
     - Free text field allowing a user to describe the configuration file in
       their own words.
       The contents of this field are not parsed or used internally.

   * - ``ListenUri``
     - The configured registry instance will listen on this address for
       incoming connections.
       This field overrides the ``-u``, and ``--listen-uri`` command line
       parameters.

   * - ``Logging``
     - Configuration of where and how logs produced by the registry are
       processed. See :ref:`Logging<sec:cfg-participant-logging>`.

       **NOTE** It is not possible to configure ``LogFromRemotes``. Using this
       value will lead to an error during registry startup.

       **NOTE** It is only possible to use the Sink types ``Stdout`` and
       ``File``. Using ``Remote`` will lead to an error during registry startup.

   * - ``DashboardUri``
     - The configured registry instance will send data to this address to show it on the dashboard
       This field overrides the ``-d``, and ``--dashboard-uri`` command line
       parameters.

       **NOTE** The default URI to use for a local SIL Kit dashboard setup is http://localhost:8082.

.. _subsec:registry-config-options:

Configuration Options
~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2

   logging-configuration
