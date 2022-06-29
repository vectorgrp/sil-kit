======================
Demos
======================

This document describes the usage of the demo projects that are
included with the Vector Integration Bus project and what their
expected output and or results are. All demo source code is located in
the Git repository in the folder Demos.

.. |UtilDir| replace:: build/Release
.. |DemoDir| replace:: build/Release
.. |SystemMonitor| replace::  |UtilDir|/IbSystemMonitor
.. |SystemController| replace::  |UtilDir|/IbSystemController

.. admonition:: Note

   All paths on the following pages are relative to the top level of
   the Git repository. Build artifacts are assumed to be located in a
   ``build`` subdirectory.
   For simplicity's sake all paths assume a Release configuration.


To build the demos, please refer to :ref:`sec:build-demos`.


.. _sec:build-demos:

Building the Demos
~~~~~~~~~~~~~~~~~~

Building the demos from within the source tree is straight forward,
just build the  ``Demos`` CMake target:
    
    cmake --build . --target Demos

The individual demos are build as a dependency.

.. admonition:: Note
   
   The distributed Demos, as packaged by CPack, are preconfigured to build against 
   a copy of the VIB binaries in ``../IntegrationBus/`` .
   This can be overriden by providing your own ``IntegrationBus`` CMake target library,
   before the demos are configured by cmake.
   Or by changing the ``find_package(IntegrationBus ... PATHS path/to/VIB)`` statement directly
   in the ``IntegrationBus-Demos/CMakeLists.txt`` directory.


.. _sec:util-can-demo:

CAN Demo
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  CAN Reader/Writer with or without network simulator
   *  -  Source location
      -  Demos/Can
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
         * network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguration to be used; 
           use ``IbConfig_DemoCan.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``CanWriter`` or 
           ``CanReader``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the SystemController to start.
   *  -  Parameter Example
      -  .. parsed-literal:: 
            
            # Creates a CAN Writer Process with the default registry URI
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # CAN Reader:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanReader

            # CAN Writer:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter

            # System Controller:
            |SystemController| CanReader CanWriter 

        For unsynchronized execution:

        .. parsed-literal:: 

            # CAN Reader:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanReader --async

            # CAN Writer:
            |DemoDir|/IbDemoCan Demos/Can/IbConfig_DemoCan.json CanWriter --async


Ethernet Demo
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Ethernet Reader / Writer with or without network simulator
   *  -  Source location
      -  Demos/Ethernet
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
         * Network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``IbConfig_DemoEthernet.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``EthernetWriter`` or 
           ``EthernetReader``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the SystemController to start.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates an Ethernet Writer Process with the default registry URI:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Ethernet Reader:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetReader

            # Ethernet Writer:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter

            # System Controller:
            |SystemController| EthernetReader Ethernet Writer

        For unsynchronized execution:

        .. parsed-literal:: 

            # Ethernet Reader:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetReader --async

            # Ethernet Writer:
            |DemoDir|/IbDemoEthernet Demos/Ethernet/IbConfig_DemoEthernet.json EthernetWriter --async

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
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
         * Network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``IbConfig_DemoLin.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``EthernetWriter`` or 
           ``EthernetReader``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a LIN Master Process with the default registry URI:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # LIN Master:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinMaster

            # LIN Slave:
            |DemoDir|/IbDemoLin Demos/Lin/IbConfig_DemoLin.json LinSlave

            # System Controller:
            |SystemController| LinSlave LinMaster
   *  -  Notes
      -  | \- The LIN demo can only run in a synchronized mode.
         | \- Both Master and Slave sleep for 500 millisecond per simulation task to slow down execution.


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
         * Network simulator (mandatory)
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``IbConfig_DemoFlexRay.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Node0`` or 
           ``Node1``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 with the default registry URI:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0
   *  -  System Example
      -  .. parsed-literal:: 

            # Network simulator (assumed to be in PATH, necessary):
            NetworkSimulator Demos/FlexRay/NetworkSimulatorConfig.json

            # System Monitor (optional):
            |SystemMonitor|

            # Node 0:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node0

            # Node 1:
            |DemoDir|/IbDemoFlexray Demos/FlexRay/IbConfig_DemoFlexray.json Node1

            # System Controller:
            |SystemController| Node0 Node1 NetworkSimulator
   *  -  Notes
      -  Starting the FlexRay cycle takes quite some time, which is accurately modeled by the NetworkSimulator. 
         It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.


Data Message Demo
~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Data Message Publish Subscribe Demo for a set of Publishers/Subscribers
   *  -  Source location
      -  Demos/DataMessage
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``IbConfig_DemoDataMessage.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``PubSub1``, ``PubSub2``, ``Subscriber1`` or 
           ``Subscriber2``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 with the default registry URI:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json PubSub1
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Publisher 1:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json PubSub1

            # Publisher 2:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json PubSub2
            
            # Subscriber 1:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json Subscriber1
            
            # Subscriber 2:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json Subscriber2

            # System Controller:
            |SystemController| PubSub1 PubSub2 Subscriber1 Subscriber2
   *  -  Notes
      -  Any combination of publishers or subscribers is applicable for this demo.

RPC Demo
~~~~~~~~~~~~~~~~~~~~


.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Remote Procedure Call Demo. The client triggers remote procedure calls on the server.
   *  -  Source location
      -  Demos/DataMessage
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>`
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``IbConfig_DemoRpc.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Server`` or 
           ``Client``.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 with the default registry URI:
            |DemoDir|/IbDemoRpc Demos/Rpc/IbConfig_DemoRpc.json Server
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Server:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json Publisher1

            # Client:
            |DemoDir|/IbDemoDataMessage Demos/DataMessage/IbConfig_DemoDataMessage.json Publisher2
            
            # System Controller:
            |SystemController| Server Client
   *  -  Notes
      -  Any combination of publishers or subscribers is usable for this demo.

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
         For convenience long command options are supported with the syntax ``--option value``
         
         #. The middleware to be used (optional); must be ``VAsio``; defaults to ``VAsio``.
             - ``--middleware VAsio``
         #. Number of simulations (optional); must be at least ``1``; defaults to ``5``.
             - ``--number-simulations NUM``
         #. Duration of the simulation in seconds (optional); must be at least ``1``; defaults to ``1``.
             - ``--simulation-duration SECONDS``
         #. Number of participants (optional); must be at least ``2``; defaults to ``4``.
             - ``--number-participants NUM``
         #. Number of messages sent per tick between each participant (optional); defaults to ``1``.
             - ``--message-count NUM``
         #. Size of the messages in bytes (optional); must be at least ``1``; defaults to ``100``.
             - ``--message-size BYTES``
         #. Registry URI (optional); defaults to ``vib://localhost:8500``.
             - ``--registry-uri URI``
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a benchmark process, which runs the same simulation (VAsio middleware, 5s duration,
            # 10 participants, 1 message of 200 bytes per participant pair per tick) a hundred times.
            |DemoDir|/IbDemoBenchmark VAsio 100 5 10 1 200 50
   *  -  Notes
      -  | \- DataPublisher / DataSubscribers are used in the participants.
         | \- The tick period is 1ms and each tick, each particpant sends the specified number of messages to every other particpant.
         | \- All participants and the VAsio registry (VAsio only) run in the same process.


Life Cycle Demo
~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Participant with or without life cycle and / or time synchronization
   *  -  Source location
      -  Demos/Lifecycle
   *  -  Requirements
      -  * :ref:`SystemController<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`SystemMonitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml>
           File name of the ParticipantConfiguration to be used;
           use ``IbConfig_DemoLifecycle.json`` for an example configuration.
         <ParticipantName>
           The name of the participant within the simulation; pauses and continues the simulation three times for five seconds if ``PauseTest``; can be anything otherwise.
         [registryUri] 
           The vib:// URI of the registry to connect to; defaults to vib://localhost:8500 (optional).
         [\-\-async]
           If timeSync flag is set, the participant will run without virtual time synchronization.
         [\-\-uncoordinated]
           If the uncoordinated flag is set, the participant will not coordinate its state transitions with other participants. 
           The state transition Running->Stopping must be triggered via a call to :cpp:func:`ILifecycleService::Stop()<ib::mw::sync::ILifecycleService::Stop()>`.
   *  -  Parameter Example
      -  .. parsed-literal::

            # Creates an Life Cycle Demo Process in the default domain 42:
            |DemoDir|/IbDemoLifecycle Demos/Lifecycle/IbConfig_DemoLifecycle.json PauseTest --coordinateStartAndStop --syncTime

   *  -  System Example
      -  .. parsed-literal::

            # System Monitor (optional):
            |SystemMonitor|

            # Life cycle with coordinated start and stop, synchronized time and running the pause testing:
            |DemoDir|/IbDemoLifecycle Demos/Lifecycle/IbConfig_DemoLifecycle.json PauseTest --coordinateStartAndStop --syncTime

            # Life cycle with synchronized time, but without coordinated start and stop (i.e., switches directly to the Running state):
            |DemoDir|/IbDemoLifecycle Demos/Lifecycle/IbConfig_DemoLifecycle.json AnotherParticipant --syncTime

            # System Controller (add NetworkSimulator as third parameter if using VIBE Network Simulator):
            |SystemController| EthernetReader Ethernet Writer

   *  -  Notes
      -  | \- The ``PauseTest`` pauses in three consecutive time-steps for five (wall-clock) seconds, starting at simulation timestamp 0.02s.
