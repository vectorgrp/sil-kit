========================================
Configuration
========================================

.. _sec:ibconfig-json:

.. toctree::
   :maxdepth: 2

   configuration-services
   tracing-configuration
   logging-configuration
   healthcheck-configuration
   extension-configuration
   middleware-configuration



The Participant Configuration File
=======================================

Simulation participants of the Vector Integration Bus can be configured via a YAML/JSON file, often
referred to as *participant configuration*, ``IbConfig.json``, or ``IbConfig.yaml``. A configuration file is optional, 
it is intended to be used to configure behavior and connections of a simulation participant that was distributed in 
binary form.
A participant configuration can be passed to a simulation when a simulation participant is created 
through :cpp:func:`CreateParticipant()<ib::CreateParticipant()>`

Configuration parameters that are specified within the participant configuration override corresponding 
programmatically defined values, i.e. the ``ParticipantName`` field of the participant configuration overrides the 
participant name that is provided through the API of the Vector Integration Bus when the participant is created. 

An ``IbConfig.yaml`` begins with some general information about the configuration file itself, followed by several 
subsections for the different services of the Vector Integration Bus.

The outline of a IbConfig.yaml file is as follows:

.. code-block:: yaml
                
    ---
    "$schema": "./ParticipantConfiguration.schema.json"
    schemaVersion: 1
    Description: Sample configuration for CAN Demonstrator
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
    - ...
    Extensions: 
    - ...
    Middleware: 
    - ...


Configuration Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - $schema
     - The location of the participant configuration schema file. The ``ParticipantConfiguration.schema.json`` is
       part of the VIB sources and can be found in the folder
       ``./IntegrationBus/source/cfg/``.

   * - schemaVersion
     - The version of the used participant configuration schema. Current version is 1.
       
   * - ParticipantName
     - The name of the simulation participant that joins the Vector Integration Bus simulation. Overrides a programmatically
       defined participant name.

   * - :doc:`CanControllers<configuration-services>`, 
       :doc:`LinControllers<configuration-services>`,
       :doc:`FlexrayControllers<configuration-services>`,
       :doc:`EthernetControllers<configuration-services>`,
       :doc:`DataPublishers<configuration-services>`,
       :doc:`DataSubscribers<configuration-services>`,
       :doc:`RpcClients<configuration-services>`,
       :doc:`RpcServers<configuration-services>`
     - These sections allow to configure bus controllers and other communication services. 

   * - :ref:`Logging<sec:cfg-participant-logging>`
     - The logger configuration for this participant.

   * - :ref:`HealthCheck<sec:cfg-participant-healthcheck>`
     - Configuration concerning soft and hard timeouts for simulation task execution.

   * - :doc:`Tracing<tracing-configuration>`
     - Describes relevant configuration aspects for the tracing features of the Vector Integration Bus. The TraceSinks
       and TraceSources used for Tracing and Replaying are defined here. 

   * - :doc:`Extensions<extension-configuration>`
     - Configuration of optional extensions to the Vector Integration Bus and where to find them.

   * - :doc:`Middleware<middleware-configuration>`
     - This optional section can be used to configure Vector VAsio, the middleware running the Vector Integration Bus.
       If this section is omitted, defaults will be used.


