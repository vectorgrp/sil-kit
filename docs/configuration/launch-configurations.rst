===================================================
Launch Configurations
===================================================

.. contents:: :local:
   :depth: 3

Overview
========================================
The LaunchConfigurations is an optional section in the IbConfig.json containing one or more
configurations that can be used with the IbLauncher to start all processes of the configured
simulation. For example, a LaunchConfiguration of the CAN demo with the name "Installation"
can be started like this:

  .. code-block:: powershell

    # Launch CAN demo:
    bin/IbLauncher.py ../IntegrationBus-Demos/Can/IbConfig_DemoCan.json -c Installation

A LaunchConfiguration comprises general information such as a required name and an optional description,
as well the mandatory ParticipantEnvironments section used to start the individual participant processes:


.. code-block:: javascript
                
    "LaunchConfigurations": [
        {
            "Name": "Installation",
            "Description": "Uses the installation binaries",

            "ParticipantEnvironments" : [ ... ],
        },
        ...
    ]


.. _sec:cfg-participant-environments:
    
ParticipantEnvironments
========================================
The mandatory section ParticipantEnvironments is used by the :ref:`IbLauncher<sec:util-launcher>`
to automatically start all processes in order and thus support the automation of simulation runs.

The following example spawns two processes of the :ref:`IbDemoCan<sec:util-can-demo>`
where one of them serves as CanWriter and the other as CanReader. Furthermore, the
:ref:`sec:util-system-controller` of the :doc:`../usage/utilities` is used in a third process
to control the simulation:

.. code-block:: javascript

    "ParticipantEnvironments": [
        {
            "Participant": "CanWriter",
            "Environment": "CustomExecutable",
            "WorkingFolder": ".",
            "Executable": "../bin/IbDemoCan",
            "Arguments": "%INTEGRATIONBUS_CONFIGFILE% CanWriter %INTEGRATIONBUS_DOMAINID%"
        },
        {
            "Participant": "CanReader",
            "Environment": "CustomExecutable",
            "WorkingFolder": ".",
            "Executable": "../bin/IbDemoCan",
            "Arguments": "%INTEGRATIONBUS_CONFIGFILE% CanReader %INTEGRATIONBUS_DOMAINID%"
        },
        {
            "Participant": "SystemController",
            "Environment": "CustomExecutable",
            "WorkingFolder": ".",
            "Executable": "../bin/IbSystemController",
            "Arguments": "%INTEGRATIONBUS_CONFIGFILE% %INTEGRATIONBUS_DOMAINID%"
        }
    ]

.. list-table:: Participant Environment Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Participant
     - The required name of the participant that is specified by this environment.
   * - Environment
     - A required environment that shall be used for this participant. Valid options
       are *CustomExecutable*, *CANoe*, *ExecutionController*.
   * - WorkingFolder
     - A working folder where the participant is executed.
       For environment *CustomExecutable* and *CANoe* this property is mandatory.
   * - Executable
     - A (relative) path to the custom executable of the given participant. This property
       is required and only valid for environment *CustomExecutable*.
   * - Arguments
     - Optional command line arguments for the executable. Only valid for the
       environment *CustomExecutable*.
   * - CANoeProject
     - A (relative) path to the CANoe configuration file (.cfg). This property is 
       required and only valid for the environment *CANoe*.
