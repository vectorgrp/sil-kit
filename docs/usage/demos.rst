======================
VIB Demos
======================

This document describes the usage of the demo projects that are
included with the Vector Integration Bus project and what their
expected output and or results are. All demo source code is located in
the GIT repository in the folder Demos.

.. |UtilDir| replace:: build/Utilities/Release/bin
.. |DemoDir| replace:: build/Demos/Release/bin
.. |SystemMonitor| replace::  |UtilDir|/IbSystemMonitor
.. |SystemController| replace::  |UtilDir|/IbSystemController

.. admonition:: Note

   All paths on the following pages are relative to the top level of
   the GIT repository. Build artifacts are assumed to be located in a
   ``build`` subdirectory.
   Utilities are build in  ``build/Utilities/<CONFIG>/bin`` where '<CONFIG>' is the current cmake build configuration. For simplicity's sake all paths assume a Release configuration.


To build the demos, please refer to :ref:`sec:build-demos`.

.. _sec:util-can-demo:

CAN Demo
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  CAN Reader/Writer with or without VIBE Network Simulator
   *  -  Source location
      -  Demos/Can
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
         * :doc:`NetworkSimulator<../vibes/networksimulator>` (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoCan configs.
         #. Name of the participant in the configuration; must be either CanWriter or CanReader.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 
            
            # Creates a CAN Writer Process in the default domain 42:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter
   *  -  System Example
      -  .. parsed-literal:: 

            # VIBE Network Simulator (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Can/IbConfig_DemoCan_NetSim.json

            # System Monitor (optional):
            |SystemMonitor| Demos/Can/IbConfig_DemoCan.json

            # CAN Reader:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanReader

            # CAN Writer:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter

            # System Controller:
            |SystemController| Demos/Can/IbConfig_DemoCan.json


Ethernet Demo
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Ethernet Reader / Writer with or without VIBE Network Simulator
   *  -  Source location
      -  Demos/Ethernet
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
         * :doc:`NetworkSimulator<../vibes/networksimulator>` (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoEthernet configs.
         #. Name of the participant in the configuration; must be either EthernetWriter or EthernetReader.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates an Ethernet Writer Process in the default domain 42:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter
   *  -  System Example
      -  .. parsed-literal:: 

            # VIBE Network Simulator (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Ethernet/IbConfig_DemoEthernet_NetSim.json

            # System Monitor (optional):
            |SystemMonitor| Demos/Ethernet/IbConfig_DemoEthernet.json

            # Ethernet Reader:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetReader

            # Ethernet Writer:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter

            # System Controller:
            |SystemController| Demos/Ethernet/IbConfig_DemoEthernet.json
   *  -  Notes
      -  | \- The writer sends Ethernet messages at a fixed rate of one message per quantum.
         | \- Both reader and writer sleep for 1 second per quantum to slow down execution.


LIN Demo
~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  LIN Master and Slave demo. The master sends and requests messages from a LIN slave.
   *  -  Source location
      -  Demos/Lin
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
         * :doc:`NetworkSimulator<../vibes/networksimulator>` (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoLin configs.
         #. Name of the participant in the configuration; must be either LinMaster or LinSlave.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a LIN Master Process in the default domain 42:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster
   *  -  System Example
      -  .. parsed-literal:: 

            # VIBE Network Simulator (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/Lin/IbConfig_DemoLin_NetSim.json

            # System Monitor (optional):
            |SystemMonitor| Demos/Lin/IbConfig_DemoLin.json

            # LIN Master:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster

            # LIN Slave:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinSlave

            # System Controller:
            |SystemController| Demos/Lin/IbConfig_DemoLin.json
   *  -  Notes
      -  | \- Both Master and Slave sleep for 1 second per quantum to slow down execution.
         | \- The master alternatively sends and requests LIN messages. It sends a message for LIN ID 17 and requests a message for LIN ID 34.
         | \- The slave is configured to trigger a callback on LIN ID 17 and replies with the String "Hello!" on LIN ID 34.


FlexRay Demo
~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  FlexRay Demo for a FlexRay cluster containing two nodes
   *  -  Source location
      -  Demos/FlexRay
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
         * :doc:`NetworkSimulator<../vibes/networksimulator>` (optional)
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be either of the two provided DemoFlexray configs.
         #. Name of the participant in the configuration; must be either Node0 or Node1.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 in the default domain 42:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0
   *  -  System Example
      -  .. parsed-literal:: 

            # VIBE Network Simulator (assumed to be in PATH, optional):
            NetworkSimulator BusSimulator Demos/FlexRay/IbConfig_DemoFlexray_NetSim.json

            # System Monitor (optional):
            |SystemMonitor| Demos/FlexRay/IbConfig_DemoFlexray.json

            # Node 0:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0

            # Node 1:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node1

            # System Controller:
            |SystemController| Demos/FlexRay/IbConfig_DemoFlexray.json
   *  -  Notes
      -  Starting the FlexRay cycle takes quite some time, which is accurately modeled by the NetworkSimulator. 
         It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.


Generic Message Demo
~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Untimed demo to send and receive Generic Messages
   *  -  Source location
      -  Demos/GenericMessage
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be the provided IbConfig_DemoGenericMessage.json config. 
         #. Name of the participant in the configuration; must be either Publisher or Subscriber.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a generic message process with for the participant Publisher:
            |DemoDir|/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Publisher
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor| Demos/GenericMessage/IbConfig_DemoGenericMessage.json

            # Generic Message Subscriber:
            |DemoDir|/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Subscriber

            # Generic Message Publisher:
            |DemoDir|/IbDemoGenericMessage Demos/GenericMessage/IbConfig_DemoGenericMessage.json Publisher

            # System Controller:
            |SystemController| Demos/GenericMessage/IbConfig_DemoGenericMessage.json
   *  -  Notes
      -  | \- Both Publisher and Subscriber sleep for 1 second per quantum to slow down execution.
         | \- The Publisher sends two topics "GroundTruth" and "VehicleModelOut" to show 
           that multiple generic message topics are created and separated from each other.


IO Port Demo
~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Sender / Receiver demo for IO values. The demo uses Analog IO, Digital IO, PWM IO, and Pattern IO.
   *  -  Source location
      -  Demos/IO
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor (optional)<sec:util-system-monitor>`
   *  -  Parameters
      -  There are up to 3 positional arguments:
         
         #. Filename of the IB Configuration to be used; must be the provided IbConfig_DemoIo.json config. 
         #. Name of the participant in the configuration; must be either IoWriter or IoReader.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates an IO-Writer process:
            |DemoDir|/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoWriter
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |DemoDir|/IbDemoPassiveSystemMonitor Demos/Io/IbConfig_DemoIo.json

            # IO Value Writer:
            |DemoDir|/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoWriter

            # IO Value Reader:
            |DemoDir|/IbDemoIo Demos/Io/IbConfig_DemoIo.json IoReader

            # System Controller:
            |SystemController| Demos/Io/IbConfig_DemoIo.json
   *  -  Notes
      -  | \- Both Writer and Reader sleep for 1 second per quantum to slow down execution.
         | \- In each quantum, all four types of IO are used in each quantum: analog, digital, PWM, and pattern IO.

.. _sec:util-benchmark-demo:

Benchmark Demo
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Parametrizable demo to benchmark the IB performance. Runs the simulation with the specified parameters a number of times and summarizes the real execution time as result.
   *  -  Source location
      -  Demos/Benchmark
   *  -  Parameters
      -  There are up to 7 positional arguments. All of them are optional and the defaults are used for the unspecified ones.
         
         #. The middleware to be used (optional); must be either ``FastRTPS`` or ``VAsio``; defaults to ``VAsio``. 
         #. Number of simulations (optional); must be at least ``1``; defaults to ``5``.
         #. Duration of the simulation in seconds (optional); must be at least ``1``; defaults to ``1``.
         #. Number of participants (optional); must be at least ``2``; defaults to ``4``.
         #. Number of messages sent per tick between each participant (optional); defaults to ``1``.
         #. Size of the messages in bytes (optional); must be at least ``1``; defaults to ``100``.
         #. Domain ID (optional); defaults to ``42``.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a benchmark process, which runs the same simulation (VAsio middleware, 5s duration,
            # 10 participants, 1 message of 200 bytes per participant pair per tick) a hundred times.
            |DemoDir|/IbDemoBenchmark VAsio 100 5 10 1 200 50
   *  -  Notes
      -  | \- Generic publisher / subscribers are used as participants.
         | \- The tick period is 1ms and each tick, each particpant sends the specified number of messages to every other particpant.
         | \- All participants, the SyncMaster and the VAsio registry (VAsio only) run in the same process.


C-API CanDemo
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Demonstrates the C language bindings for two can controllers.
   *  -  Source location
      -  Demos/CCan
   *  -  Parameters
      -  There are up two positional arguments:

         #. Filename of the IB Configuration to be used; must be the provided VirtualCanCable1_VAsio.json config.
         #. Name of the participant in the configuration; must be "CANoe"

   *  -  Parameter Example
      -  .. parsed-literal:: 
            # starting IbRegistry is required for VAsio to work

            |DemoDir|/IbDemoCCan VirtualCanCable1_VAsio.json  CANoe

C-API EthernetDemo
~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Demonstrates the C language bindings for two ethernet controllers.
   *  -  Source location
      -  Demos/CEthernet
   *  -  Parameters
      -  There are up two positional arguments:

         #. Filename of the IB Configuration to be used; must be the provided IbConfig_DemoEthernet_SingleParticipant.json.
         #. Name of the participant in the configuration; must be "EthernetReaderWriter"

   *  -  Parameter Example
      -  .. parsed-literal:: 
            # starting IbRegistry is required for VAsio to work

            |DemoDir|/IbDemoCEthernet IbConfig_DemoEthernet_SingleParticipant.json EthernetReaderWriter
