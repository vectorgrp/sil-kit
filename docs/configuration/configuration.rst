========================================
Configuration
========================================

.. |ProductName| replace:: SIL Kit

.. _sec:sil-kit-config-yaml:

The Vector |ProductName| provides optional configuration to enhance flexibility and control through use of YAML files.
This allows users to modify behavior without recompiling source code.
Further, it allows configuration of services which are inactive by default, such as tracing and logging.
Different configuration options are available for |ProductName| participants and the |ProductName| Registry.

.. contents::
   :local:
   :depth: 1

.. _sec:participant-config:

The Participant Configuration File
=======================================

Participants are primarily configured by developers in source code using the |ProductName| API.
However, the following scenarios are conceivable in which some flexibility is useful even after compile-time:

- A simulation participant is distributed in binary form to users. 
  In order to integrate such participants into simulations, a user must be able to post-configure some aspects of the participant to make it fit, particularly its properties, behavior and how it interconnects within a simulation.
- A simulation consists of multiple participants that share the same behavior. 
  A user would have to ask developers to compile these participants from the same sources multiple times, with only the participant names differing.
- Developers or users want to temporarily enable debugging features without the need or the ability to recompile the sources, for example logging, tracing, and health checking.

To cover these scenarios, |ProductName| participants can be modified by *participant configuration files*. 
This is done by creating a participant configuration object from a given file in YAML or JSON format and passing the object as an argument when a participant is created:

.. code-block:: c++

  auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);
  auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

Alternatively, the configuration object can be created directly from string:

.. code-block:: c++

  const std::string participantConfigText = R"(
  Description: My participant configuration
  Logging:
      Sinks:
      - Type: Stdout
        Level: Info
  )";
  auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(participantConfigText);
  auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

Participant configurations allow to override a subset of parameters which are configurable via the |ProductName| API.
Configuration parameters that are specified within the participant configuration override corresponding programmatically defined values. 
For example, the ``ParticipantName`` field overrides the participant name that is provided through the API when a participant is created, namely :cpp:func:`CreateParticipant(..., const std::string& participantName, ...)<SilKit::CreateParticipant()>`. 
This gives users the ability to run a simulation with multiple instances of a participant from a single implementation.

.. admonition:: Note

    It is recommended that configurations are specified in the source code where possible.
    That way, an empty configuration file can be provided for most simulation runs.
    However, a process that creates a |ProductName| participant should also accept a configuration file
    such that a user is able to reconfigure names or network connections without recompiling the application.

.. admonition:: Note

    Many IDEs automatically support participant configuration schema support when the participant configuration file ends with the suffix ``.silkit.json/yaml``.

A participant configuration file is written in YAML syntax according to a specified schema. 
It starts with the ``SchemaVersion``, the ``Description`` for the configuration and the ``ParticipantName``. 
This is followed by further sections for ``Includes``, ``Middleware``, ``Logging``, ``HealthCheck``, ``Tracing``, ``Extentions`` and sections for the different services of the |ProductName|.
The outline of a participant configuration file is as follows:

.. code-block:: yaml
                
    "$schema": "./ParticipantConfiguration.schema.json"
    SchemaVersion: 1
    Description: Sample configuration with all root nodes
    ParticipantName: Participant1
    Includes: 
      ...
    Middleware: 
      ...
    Logging: 
      ...
    HealthCheck: 
      ...
    Tracing:
      ...
    Extensions: 
      ...
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

.. _subsec:participant-config-overview:

Overview
~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - ``"$schema"``
     - File path to the participant configuration schema. 
       The ``ParticipantConfiguration.schema.json`` is part of the |ProductName| sources and can be found in the folder ``./SilKit/source/config/``.

   * - ``SchemaVersion``
     - The version of the used participant configuration schema. Current version is 1.

   * - ``Description``
     - User defined description for the configuration.

   * - ``ParticipantName``
     - The name of the simulation participant that joins the |ProductName| simulation. 
       Overrides a programmatically defined participant name.

   * - :ref:`Includes<sec:cfg-participant-includes>`
     - This can be used to include other participant configuration files.

   * - :ref:`Logging<sec:cfg-participant-logging>`
     - The logger configuration for this participant.

   * - :ref:`HealthCheck<sec:cfg-participant-healthcheck>`
     - Configuration concerning soft and hard timeouts for simulation task execution.

   * - :ref:`Tracing<sec:cfg-participant-tracing>`
     - Configuration of experimental tracing and replay functionality.

   * - :ref:`Extensions<sec:cfg-participant-extensions>`
     - Configuration of optional extensions to the |ProductName| and where to find them.

   * - :ref:`Middleware<sec:cfg-participant-middleware>`
     - This optional section can be used to configure the middleware running the |ProductName|.
       If this section is omitted, defaults will be used.

   * - :ref:`CanControllers<sec:cfg-participant-can>`
     - Configure CAN controllers.

   * - :ref:`LinControllers<sec:cfg-participant-lin>`
     - Configure LIN controllers.

   * - :ref:`EthernetControllers<sec:cfg-participant-ethernet>`
     - Configure Ethernet controllers.

   * - :ref:`FlexrayControllers<sec:cfg-participant-flexray>`
     - Configure Flexray controllers.

   * - :ref:`DataPublishers<sec:cfg-participant-data-publishers>`
     - Configure data publishers.

   * - :ref:`DataSubscribers<sec:cfg-participant-data-subscribers>`
     - Configure data subscribers.

   * - :ref:`RpcServers<sec:cfg-participant-rpc-servers>`
     - Configure RPC servers.

   * - :ref:`RpcClients<sec:cfg-participant-rpc-clients>`
     - Configure RPC clients.

Configuration Options
~~~~~~~~~~~~~~~~~~~~~
.. toctree::
   :maxdepth: 1

   services-configuration
   includes-configuration
   logging-configuration
   healthcheck-configuration
   tracing-configuration
   extension-configuration
   middleware-configuration

.. _sec:registry-config:

The Registry Configuration File
===============================

An instance of the |ProductName| Registry (``sil-kit-registry(.exe)``) can be configured via a YAML file.
The configuration file is optional and overrides the settings specified on the command line.
It also allows extended configuration, beyond what the command line allows, particularly for logging.

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

       **NOTE** The default URI to use for a local |ProductName| dashboard setup is http://localhost:8082.

.. _subsec:registry-config-options:

Configuration Options
~~~~~~~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 2

   logging-configuration


API and Data Type Reference
---------------------------

.. doxygenfunction:: SilKit::Config::ParticipantConfigurationFromFile

.. doxygenfunction:: SilKit::Config::ParticipantConfigurationFromString

