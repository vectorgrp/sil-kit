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

        "Switches": [ ... ],

        "Links": [ ... ],

        "NetworkSimulators": [ ... ],

        "TimeSync": { ... }

    }



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
            "IsSyncMaster": true,

            "ParticipantController": {
                ...
            },
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
    ]


.. list-table:: Participant Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description

   * - Name
     - The unique name of the participant (mandatory)

   * - Description
     - A human readable description of the participant (optional)

   * - IsSyncMaster
     - A boolean flag that identifies the participant as synchronization master.

   * - :ref:`ParticipantController<sec:cfg-participant-controller>`
     - The participant controller enables synchronization with other
       participants.
       
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
     - A list of PWM input port names, e.g., [ "PWMI1", "PWMI2", "PWMI3"].
   * - Pattern-In
     - A list of pattern input port names, e.g., [
       "PATTERN-IN-1", "PATTERN-IN-2", "PATTERN-IN-3"].

       
   * - NetworkSimulators
     - A list NetworkSimulator names simulated by this participant.



.. _sec:cfg-participant-controller:
       
ParticipantController
----------------------------------------

The optional ParticipantController section enables synchronization with other
participants. If no ParticipantController section is specified, the participant
does not synchronize time with other participants and it does not contribute to
the global system state. The ParticipantController also allows specifying soft
and hard limits for the execution of each simulation task.

The following example enables DiscreteTime synchronization, with a soft
execution limit of 1.010 seconds and a hard limit of 1.5 seconds:

.. code-block:: javascript

    "ParticipantController": {
        "SyncType": "DiscreteTime",
        
        "ExecTimeLimitSoftMs": 1010,
        "ExecTimeLimitHardMs": 1500
    }

.. list-table:: Logger Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SyncType
     - The synchronization mechanism used by the participant
       (mandatory). Currently supported options are *DistributedTimeQuantum*,
       *TimeQuantum*, *DiscreteTime*, *DiscreteTimePassive*. If a participant
       should not take part in the actual simulation (e.g., SystemMonitor), it
       must not specify a ParticipantController at all.
   * - ExecTimeLimitSoftMs
     - The (optional) soft limit for the execution of a simulation task given in
       miliseconds. If the simulation task does not finish within this limit, a warning
       message is logged. This limit is checked for each execution of the simulation
       task. 
   * - ExecTimeLimitHardMs
     - The (optional) hard limit for the execution of a simulation task given in
       miliseconds. If the simulation task does not finish within this limit, an
       error message is logged and the participant switches to the Error state,
       which suspends further execution of the simulation.

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
       remote sinks. Log messages received from other participants are only 
       sent to local sinks, i.e., *Stdout* and *File*



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
       with a lower log level are ignored. Valid options are *Critical*,
       *Error*, *Warn*, *Info*, *Debug*, *Trace*, and *Off*.
   * - Logname
     - The logname determines the filename used by sinks of type *File*. The
       resulting filename is <Logname>_<iso-timestamp>.txt.



.. _sec:cfg-participant-ethernet:

EthernetControllers
----------------------------------------
The Ethernet controller configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`.

.. code-block:: javascript
    
  "EthernetControllers": [
      {
          "Name": "ETH0",
          "MacAddr": "00:08:15:ab:cd:ef"
      },
      {
          "Name": "ETH1",
          "MacAddr": "00:08:15:ab:cd:f0",
          "PcapFile": "pcap_output_trace.pcap"
      }
  ]


.. list-table:: Ethernet Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the Ethernet Controller
   * - MacAddr
     - The colon-separated Ethernet MAC address
   * - PcapFile
     - Name of the file capturing the PCAP trace
   * - PcapPipe
     - Name of the named pipe capturing the PCAP trace.
       Execution is suspended until the pipe is opened for reading by another process.



.. _sec:cfg-participant-flexray:

FlexRayControllers
----------------------------------------
The Ethernet controller configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`.

.. code-block:: javascript
    
  "FlexRayControllers": [
      {
          "Name": "FlexRay1",
          "ClusterParameters": {
              "gColdstartAttempts": 8,
              ...
          },
          "NodeParameters": {
              "pChannels": "AB",
              ...
          }
      }
  ]


.. list-table:: FlexRay Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the FlexRay Controller
   * - ClusterParameters
     - Allows to configure cluster specific settings.
   * - NodeParameters
     - Allows to configure node specific settings.
   * - TxBufferConfigs
     - Allows to configure TXBuffers by specifying "channels" (A, B, AB, None), 
       "slotId", "offset", "repetition", "PPindicator", "headerCrc" 
       and "transmissionMode" (SingleShot, Continuous).



.. _sec:cfg-participant-genericpublisher:

GenericPublishers
----------------------------------------
The Generic Publisher configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`.

.. code-block:: javascript
    
    "GenericPublishers": [
        {
            "Name": "VehicleModelOut",
            "Protocol": "ROS",
            "DefinitionUri": "file://./vehicle-model-out.msg"
        },
        ...
    ]

.. list-table:: Generic Publisher Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - Name of the Generic Message Publisher
   * - Protocol
     - Specifies the protocol ("Undefined", "ROS", "SOME/IP").
   * - DefinitionUri
     - Location of the corresponding message definition file. 
       Relative paths are resolved relative to the location of the IbConfig file.



.. _sec:cfg-participant-digital-out:

Digital-Out (Ports)
----------------------------------------
The Digital-Out Ports configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`. The name and initial state of the participant's 
digital output port instances can be configured in this section:

.. code-block:: javascript

  "Digital-Out": [
      { "DO-Port-Name": false }
  ]

A Digital-Out Port is specified by giving the name and initial state.



.. _sec:cfg-participant-analog-out:

Analog-Out (Ports)
----------------------------------------
The Analog-Out Ports configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`. The name and initial behavior of the participant's 
analog output port instances can be configured in this section:

.. code-block:: javascript

  "Analog-Out": [
      { "AO-Port-Name": { "value": 7.3, "unit": "V" } }
  ]


.. list-table:: Analog-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - value
     - The initial voltage value
   * - unit
     - The unit of the voltage value ("mV", "V", "kV")



.. _sec:cfg-participant-pwm-out:

Pwm-Out (Ports)
----------------------------------------
The Pwm-Out Ports configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`. The name and initial behavior of the participant's 
pulse-width modulation output port instances can be configured in this section:

.. code-block:: javascript

  "Pwm-Out": [
    { "PWM-Port-Name": { "freq": { "value": 2.5, "unit": "Hz" }, "duty": 0.4 } }
  ]


.. list-table:: Pwm-Out Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - freq
     - The initial frequency is specified by its unit ("Hz", "kHz", "MHz", "GHz", "THz") and value.
   * - duty
     - The duty cycle specifies the percentage of time of each cycle that the signal stays in the
       active state. The value range is between 0 (always off) and 1 (always on)



.. _sec:cfg-participant-pattern-out:

Pattern-Out (Ports)
----------------------------------------
The Pattern-Out Ports configuration is part of the :ref:`participant
configuration<sec:cfg-participant>`.

.. code-block:: javascript
  
  "Pattern-Out": [
      { "Pattern-Port-Name": "626565702d62656570" }
  ]


The pattern-out port instances are specified by giving their name 
and a hexadecimal pattern string.



.. _sec:cfg-switches:

Switches
========================================
The Switches section describes the Ethernet Switches that can be simulated
by the :ref:`Network Simulators<sec:cfg-network-simulators>`. 
In simulations without Network Simulator, this section is not needed.

.. code-block:: javascript

  "Switches": [
      {
          "Name": "FrontSwitch",
          "Description":  "Located in the front of the car",
          "Ports": [
              {
                  "Name": "Port0",
                  "VlanIds": [1]
              },
              ...
          ]
      },
      ...
  ]

.. list-table:: Switch Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the network switch
   * - Description
     - A human readable description of the switch
   * - Ports
     - List of the Ports of the switch. Each port should be assigned a "Name" and
       a list of "VlanIds" can be specified.



.. _sec:cfg-links:

Links
========================================
The Links section of the SimulationSetup configuration describes how
the components of the simulation are connected.

.. code-block:: javascript

  "Links": [
    {
        "Name": "CAN1",
        "Endpoints": [
            "Participant1/CAN1",
            "Participant2/CAN1",
            ...
        ]
    },
    ...
  ]

.. list-table:: Link Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the link
   * - Endpoints
     - List of endpoints (can be Participants' Controllers, IO Ports and Switch Ports)
       that are connected to the link.



.. _sec:cfg-network-simulators:

NetworkSimulators
========================================
The Network Simulators section of the SimulationSetup allows to configure 
which `Links`_ and `Switches`_ should be simulated by a NetworkSimulator. 
This section is optional.

.. code-block:: javascript

  "NetworkSimulators": [
      {
          "Name": "Ethernet-Simulator",
          "SimulatedLinks": [
              "FS-Port0",
              "FS-Port1",
              ...
          ],
          "SimulatedSwitches": [
              "FrontSwitch",
              ...
          ]
      },
      ...
  ],

.. list-table:: NetworkSimulator Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the simulator
   * - SimulatedLinks
     - List of links that should be simulated by this network simulator.
   * - SimulatedSwitches
     - List of switches that should be simulated by this network simulator.



.. _sec:cfg-time-sync:

TimeSync
========================================
The TimeSync section of the SimulationSetup configuration allows to configure
how participants should be synchronized.

.. code-block:: javascript

  "TimeSync": {
      "SyncPolicy": "Loose",
      "TickPeriodNs": 1000000
  }

.. list-table:: TimeSync Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - SyncPolicy
     - The time sync policy that is used. Allowed values are "Loose" and "Strict".
       This setting determines whether to wait until data is delivered to other participants 
       (=Strict) or not (=Loose). The default is Loose.
   * - TickPeriodNs
     - Tick period in nano seconds for DiscreteTime synchronization.
