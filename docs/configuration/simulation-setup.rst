===================================================
Simulation Setup
===================================================

.. contents:: :local:
   :depth: 3

Overview
========================================

The SimulationSetup is a mandatory section in the IbConfig.json. It configures
all the participants of an VIB simulation and determines how the participants
are interconnected.

The SimulationSetup comprises five sections, most of which are optional. The
general outline is as follows:

   
.. code-block:: javascript
                
    "SimulationSetup": {

        "Participants": [ ... ],

        "EthernetSwitches": [ ... ],

        "Links": [ ... ],

        "NetworkSimulators": [ ... ],

        "TimeSync": { ... }

    },


.. _sec:cfg-participant:
    
Participants
========================================

The Participants section is a list of two or more participant
configuration. Each participant configuration contains general information,
e.g., its name and a description, the controller instances used to communication
with other participants, as well as configuration for synchronization and
logging.

.. code-block:: javascript
                
    "Participants": [
        {
            "Name": "EthernetWriter",
            "Description": "Demo Writer",
            "SyncType": "DiscreteTime",
            "IsSyncMaster": true,
            "Logger": {
                ...
            },

            "CanControllers": [ ... ],
            "LinControllers": [ ... ],
            "EthernetControllers": [ ... ],
            "FlexRayControllers": [ ... ],
            
            "GenericPublishers": [ ... ],
            "GenericSubscribers": [ ... ],

            "Digital-Out": [ ... ],
            "Analog-Out": [ ... ],
            "Pwm-Out": [ ... ],
            "Pattern-Out": [ ... ],

            "Digital-In": [ ... ],
            "Analog-In": [ ... ],
            "Pwm-In": [ ... ],
            "Pattern-In": [ ... ]


            "NetworkSimulators": [ ... ]
        },
        {
            "Name": "Participant2",
            ...
        },
    ],


.. list-table:: Participant Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Name
     - The unique name of the participant (mandatory)

   * - Description
     - A human readable description of the participant (optional)

   * - SyncType
     - The synchronization mechanism used by the participant
       (mandatory). Currently supported options are *DistributedTimeQuantum*,
       *TimeQuantum*, *DiscreteTime*, *DiscreteTimePassive*,
       *Unsynchronized*. If a participant does not take part in the actual
       simulation, it should be configured as *Unsynchronized*. This is, for
       example, the case for the SystemController or the SystemMonitor.

   * - IsSyncMaster
     - A boolean flag that identifies the participant as synchronization master.

   * - :ref:`Logger<sec:cfg-participant-logger>`
     - The logger configuration for this participant.

   * - CanControllers
     - A list of CAN controller names, e.g., [ "CAN1", "CAN2, "CAN3" ]
   * - LinControllers
     - A list of LIN controller instances, e.g., [ "LIN1", "LIN2", "LIN3" ]
   * - :ref:`EthernetControllers<sec:cfg-participant-ethernet>`
     - A list of Ethernet controller configurations
   * - :ref:`FlexRayControllers<sec:cfg-participant-flexray>`
     - A list of FlexRay controller configurations

   * - :ref:`GenericPublishers<sec:cfg-participant-genericpublisher>`
     - A list of GenericMessage publisher configurations
   * - GenericSubscribers
     - A list of GenericMessage subscribers names

   * - :ref:`Digital-Out<sec:cfg-participant-digital-out>`
     - A list of more digital output port definitions
   * - :ref:`Analog-Out<sec:cfg-participant-analog-out>`
     - A list of analog output port instances
   * - :ref:`Pwm-Out<sec:cfg-participant-pwm-out>`
     - A list of PWM output port instances
   * - :ref:`Pattern-Out<sec:cfg-participant-pattern-out>`
     - A list of pattern output port instances

   * - Digital-In
     - A list of digital input port names, e.g., [ "DI1", "DI2", "DI3"].
   * - Analog-In
     - A list of analog input port names, e.g., [ "AI1", "AI2", "AI3"].
   * - Pwm-In
     - A list of PWM input port names, e.g., [ "PWMI1", "PWMI2",
       "PWMI3"].
   * - Pattern-In
     - A list of pattern input port names, e.g., [
       "PATTERN-IN-1", "PATTERN-IN-2", "PATTERN-IN-3"].

       
   * - NetworkSimulators
     - A list NetworkSimulator names simulated by this participant.


.. _sec:cfg-participant-logger:
       
Logger
----------------------------------------

The Logger configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`, which allows individual logging behavior
per participant. The Logger uses so called sinks to store log messages and
multiple sinks can be configured at the same time. For example, to send log
messages with log level Debug or higher to a remote logger and write Trace level
logs to a file, the following configuration could be used:

.. code-block:: javascript
                
   "Logger": {
       "Sinks": [
           {
               "Type": "Remote",
               "Level": "Debug"
           },
           {
               "Type": "File",
               "Level": "Trace"
               "Logname": "ParticipantLog"
           }
       ]
   }

   
.. list-table:: Logger Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Sinks
     - A list of logger sink :ref:`configurations<sec:cfg-participant-logsink>`
   * - FlushLevel
     - The log level at which flushes are triggered.
   * - LogFromRemotes
     - A boolean flag whether to log messages from other participants with
       remote sinks. Log messages received from other participants are only sent to local sinks, i.e., *Stdout* and *File*


.. _sec:cfg-participant-logsink:
       
.. list-table:: Sink Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Type
     - The sink type determines where the log messages are stored or sent
       to. Valid options are *Stdout*, *File*, and *Remote*. Sinks of type
       *Remote* send the log messages over the underlying middleware. Note that
       this can result in a significant amount of traffic, which can impact the
       simulation performance, in particular when using a low log level.
   * - Level
     - The minimum log level of a message to be logged by the sink. All messages
       with a lower log level are ignored. Valid options are *Critical*, *Warn*,
       *Info*, *Debug*, *Trace*, and *Off*.
   * - Logname
     - The logname determines the filename used by sinks of type *File*. The
       resulting filename is <Logname>_<iso-timestamp>.txt.


.. _sec:cfg-participant-ethernet:

Ethernet Controllers
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/Ethernet/IbConfig_DemoEthernet.json)

.. list-table:: Ethernet Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - TBD
   * - MacAddr
     - TBD

     
.. _sec:cfg-participant-flexray:

FlexRay Controllers
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/FlexRay/IbConfig_DemoFlexray.json)

.. list-table:: FlexRay Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - TBD
   * - ClusterParameters
     - TBD
   * - NodeParameters
     - TBD
   * - TxBufferConfigs
     - TBD

.. _sec:cfg-participant-genericpublisher:

Generic Publishers
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/GenericMessage/IbConfig_DemoGenericMessage.json)

.. list-table:: Generic Publisher Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - TBD
   * - Protocol
     - TBD
   * - DefinitionUri
     - TBD


.. _sec:cfg-participant-digital-out:

Digital-Out Ports
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/IO/IbConfig_DemoIo.json)

.. list-table:: Digital-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - <Name>
     - <Initial Value>


.. _sec:cfg-participant-analog-out:

Analog-Out Ports
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/IO/IbConfig_DemoIo.json)

.. list-table:: Analog-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - <Name>
     - <Initial Value and Unit>

       
.. _sec:cfg-participant-pwm-out:

Pwm-Out Ports
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/IO/IbConfig_DemoIo.json)

.. list-table:: Pwm-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - <Name>
     - <Initial Frequency and Duty cycle>

       
.. _sec:cfg-participant-pattern-out:

Pattern-Out Ports
----------------------------------------

TBD: short intro

TBD: short example (based on: Demos/IO/IbConfig_DemoIo.json)


.. list-table:: Pattern-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - <Name>
     - <Initial Frequency and Duty cycle>

       
       
        
Switches
========================================

This section is optional.

TBD: short intro

TBD: short example (based on: Demos/Ethernet/IbConfig_DemoEthernet_NetSim.json)


Links
========================================

TBD: short intro

TBD: short example (based on: Demos/Ethernet/IbConfig_Can.json)


NetworkSimulators
========================================

This section is optional.

TBD: short intro

TBD: short example (based on: Demos/FlexRay/IbConfig_DemoFlexray_NetSim.json)


TimeSync
========================================

This section is optional.

TBD: short intro

TBD: short example 
