========================================
VIB Configuration
========================================

.. toctree::
   :maxdepth: 2

   simulation-setup
   middleware-configuration
   launch-configurations

   
The IbConfig.json File
----------------------------------------

The Vector Integration Bus is configured via a json config file, which is often
referred to as IbConfig.json. The IbConfig.json begins with some general
information about the configuration file itself followed by three subsections.

The outline of a IbConfig.json file is as follows:

.. code-block:: javascript
                
    {
        "$schema": "./IbConfig.schema.json",
    
        "ConfigVersion": "0.0.1",
        "ConfigName": "CanDemo",
        "Description": "Sample configuration for CAN without NetworkSimulator",
    
        "SimulationSetup": {
            ...
        },
    
        "MiddlewareConfig": {
            ...
        },
    
        "LaunchConfigurations": [
            ...
        ]
    
    }


Configuration Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 15 85
   :header-rows: 1

   * - Setting Name
     - Description

   * - $schema
     - The location of the IbConfig schema file. The IbConfig.schema.json is
       part of the VIB sources and can be found in the folder
       "./IntegrationBus/source/cfg/".
       
   * - ConfigVersion
     - The version of the config file, e.g., "1.0.0"

   * - ConfigName
     - The name of the configuration

   * - Description
     - A verbatim description of the configuration intended to help a test engineer identifying a particular configuration. (optional)

   * - :doc:`SimulationSetup<simulation-setup>`
     - This mandatory section contains all necessary information to setup a
       simulation with the Integration Bus. It configures the Integration Bus
       participants, how they communicate with each other, and how they
       synchronize. Furthermore, VIBE components such as the NetworkSimulator
       can be configured here as well.

   * - :doc:`MiddlewareConfig<middleware-configuration>`
     - This optional section can be used to select a particular middleware and
       configure particular details of the Fast-RTPS or Vector VAsio
       middleware. If this section is omitted, Fast-RTPS will be used.

   * - :doc:`LaunchConfigurations<launch-configurations>`
     - This optional section contains information to start all necessary
       processes to perform the configured simulation. It is only used by the
       IbLauncher.

