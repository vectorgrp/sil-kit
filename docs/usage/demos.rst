======================
Demos
======================

This document describes the usage of the demo projects that are
included with the Vector SIL Kit project and what their
expected output and or results are. All demo source code is located in
the Git repository in the folder Demos.

.. |UtilDir| replace:: ./SilKit/bin
.. |DemoDir| replace:: ./SilKit/bin
.. |Monitor| replace::  |UtilDir|/sil-kit-monitor
.. |Registry| replace::  |UtilDir|/sil-kit-registry
.. |SystemController| replace::  |UtilDir|/sil-kit-system-controller

.. admonition:: Note

   All paths on this page are relative to the top level of
   the pre-built Vector SIL Kit packages.


To build the demos, please refer to :ref:`sec:build-demos`.


.. _sec:build-demos:

Building the Demos
~~~~~~~~~~~~~~~~~~

This descriptions refers to the package structures as provided within the `pre-built Vector SIL Kit releases <https://github.com/vectorgrp/sil-kit/releases>`_.
It is not directly applicable for building the demos from source.

For building the demos, cmake has to be installed on your system and a corresponding cpp compiler has to be available.

Building the demos from a pre-built Vector SIL Kit is straight forward,
just run the following commands from within the "SilKit-Demos" directory::
    
    mkdir build
    cd build
    cmake ..
    cmake --build .

The individual demos are then built into ./SilKit/bin in the root directory of the package.

.. admonition:: Note
   
   The distributed Demos, as packaged by CPack, are preconfigured to build against 
   a copy of the SIL Kit binaries in ``../SilKit/``.
   This can be overridden by providing your own ``SilKit`` CMake target library,
   before the demos are configured by CMake.
   Or by changing the ``find_package(SilKit ... PATHS path/to/SilKit)`` statement directly
   in the ``./SilKit-Demos/CMakeLists.txt`` directory.


.. _sec:util-can-demo:

CAN Demo
~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  -  Abstract
      -  CAN Reader/Writer with or without network simulator
   *  -  Source location
      -  ./SilKit-Demos/Can
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
         * SIL Kit Network Simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoCan.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``CanWriter`` or 
           ``CanReader``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the |SystemController| to start.
   *  -  Parameter Example
      -  .. parsed-literal:: 
            
            # Creates a CAN Writer Process with the registry's default URI
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|
            
            # Monitor (optional):
            |Monitor|

            # CAN Reader:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader

            # CAN Writer:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter

            # System Controller:
            |SystemController| CanReader CanWriter 

        For unsynchronized execution:

        .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # CAN Reader:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader --async

            # CAN Writer:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter --async

   *  -  Notes
      -  | \- The writer sends CAN frames at a fixed rate of one frame per simulation step (1ms).
         | \- Both reader and writer sleep for 1 second per quantum to slow down execution.

Ethernet Demo
~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Ethernet Reader / Writer with or without network simulator
   *  -  Source location
      -  ./SilKit-Demos/Ethernet
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
         * SIL Kit Network Simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoEthernet.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``EthernetWriter`` or 
           ``EthernetReader``.
         [RegistryUri] 
           The ``silkit://`` URI of the registry to connect to; defaults to ``silkit://localhost:8500`` (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the |SystemController| to start.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates an Ethernet Writer Process with the registry's default URI:
            |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Monitor (optional):
            |Monitor|

            # Ethernet Reader:
            |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetReader

            # Ethernet Writer:
            |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetWriter

            # System Controller:
            |SystemController| EthernetReader EthernetWriter

        For unsynchronized execution:

        .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Ethernet Reader:
            |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetReader --async

            # Ethernet Writer:
            |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetWriter --async

   *  -  Notes
      -  | \- The writer sends Ethernet frames at a fixed rate of one frame per simulation step (1ms).
         | \- Both reader and writer sleep for 1 second per simulation step to slow down execution.


LIN Demo
~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  LIN Master and Slave demo. The master sends and requests messages from a LIN slave.
   *  -  Source location
      -  ./SilKit-Demos/Lin
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
         * SIL Kit Network Simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoLin.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``EthernetWriter`` or 
           ``EthernetReader``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the |SystemController| to start.
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a LIN Master Process with the registry's default URI:
            |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinMaster
   *  -  System Example
      -  For synchronized execution:

         .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Monitor (optional):
            |Monitor|

            # LIN Master:
            |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinMaster

            # LIN Slave:
            |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinSlave

            # System Controller:
            |SystemController| LinSlave LinMaster

         For unsynchronized execution:

         .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # LIN Master:
            |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinMaster --async

            # LIN Slave:
            |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinSlave --async

   *  -  Notes
      -  | Both Master and Slave sleep for a hort duration per simulation step to slow down execution.


FlexRay Demo
~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  FlexRay Demo for a FlexRay cluster containing two nodes
   *  -  Source location
      -  ./SilKit-Demos/FlexRay
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>`
         * SIL Kit Network Simulator (mandatory)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoFlexRay.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Node0`` or 
           ``Node1``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 with the registry's default URI:
            |DemoDir|/SilKitDemoFlexRay ./SilKit-Demos/FlexRay/DemoFlexRay.silkit.yaml Node0
   *  -  System Example
      -  .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Network Simulator (assumed to be in PATH, necessary):
            sil-kit-network-simulator ./SilKit-Demos/FlexRay/NetworkSimulatorConfig.yaml

            # Monitor (optional):
            |Monitor|

            # Node 0:
            |DemoDir|/SilKitDemoFlexRay ./SilKit-Demos/FlexRay/DemoFlexRay.silkit.yaml Node0

            # Node 1:
            |DemoDir|/SilKitDemoFlexRay ./SilKit-Demos/FlexRay/DemoFlexRay.silkit.yaml Node1

            # System Controller:
            |SystemController| Node0 Node1 NetworkSimulator
   *  -  Notes
      -  Starting the FlexRay cycle takes quite some time, which is accurately modeled by the SIL Kit Network Simulator.
         It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.


.. _sec:util-pubsub-demo:

Publish/Subscribe Demo
~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Data Message Publish/Subscribe Demo for a set of Publishers/Subscribers
   *  -  Source location
      -  ./SilKit-Demos/PubSub
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoPubSub.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Publisher`` or 
           ``Subscriber``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the |SystemController| to start.

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a publisher with the registry's default URI:
            |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Publisher
   *  -  System Example
      -  .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Monitor (optional):
            |Monitor|

            # Publisher:
            |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Publisher

            # Subscriber:
            |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Subscriber

            # System Controller:
            |SystemController| Publisher Subscriber

         For unsynchronized execution:

         .. parsed-literal::

            # Registry (if not already running):
            |Registry|

            # Publisher:
            |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Publisher --async

            # Subscriber:
            |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Subscriber --async

   *  -  Notes
      -  The publisher and subscriber show how to serialize/deserialize different kinds of data with the built-in :doc:`Data Serialization API</api/serdes>`.

RPC Demo
~~~~~~~~~~~~~~~~~~~~


.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Remote Procedure Call Demo. The client triggers remote procedure calls on the server.
   *  -  Source location
      -  ./SilKit-Demos/Rpc
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
         * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.yaml> 
           File name of the participant configuration to be used; 
           use ``DemoRpc.silkit.yaml`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Server`` or 
           ``Client``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
         [\-\-async] 
           If async flag is set, the participant will join the simulation unsynchronized and it will not need
           the |SystemController| to start.

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates an RPC server process with the registry's default URI:
            |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Server
   *  -  System Example
      -  .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|

            # Monitor (optional):
            |Monitor|

            # Server:
            |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Server

            # Client:
            |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Client

            # System Controller:
            |SystemController| Server Client

         For unsynchronized execution:

         .. parsed-literal::

            # Registry (if not already running):
            |Registry|

            # Server:
            |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Server --async

            # Client:
            |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Client --async

   *  -  Notes
      -  ``Client`` participant has two RPC clients which call the ``Add100`` and ``Sort`` functions on the ``Server`` participant's two RPC servers.

.. _sec:util-benchmark-demo:

Benchmark Demo
~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Benchmark Demo. Used for evaluating SIL Kit performance of publish/subscribe communication.
   *  -  Source location
      -  ./SilKit-Demos/Benchmark
   *  -  Requirements
      -  None (The demo starts its own instance of the registry and system controller).
   *  -  Positional parameters
      -  [numberOfSimulationRuns]
           Sets the number of simulation runs to perform.
         [simulationDuration]
           Sets the virtual simulation duration <S>.
         [numberOfParticipants]
           Sets the number of simulation participants <N>.
         [messageCount]
           Sets the number of messages <M> to be send in each simulation step.
         [messageSizeInBytes]
           Sets the message size <B>.
         [registryURi] 
           The URI of the registry to start.
   *    - Optional parameters
        - --help
            Show the help message.
          --registry-uri
            The URI of the registry to start. Default: silkit://localhost:8500
          --message-size
            Sets the message size <B> in bytes. Default: 1000
          --message-count
            Sets the number of messages <M> to be send in each simulation step. Default: 50
          --number-participants
            Sets the number of simulation participants <N>. Default: 2
          --number-simulation-runs
            Sets the number of simulation runs to perform. Default: 4
          --simulation-duration
            Sets the simulation duration <S> (virtual time). Default: 1s
          --configuration 
            Path and filename of the participant configuration YAML file. Default: empty
          --write-csv
            Path and filename of CSV file with benchmark results. Default: empty
   *  -  Parameter Example
      -  .. parsed-literal:: 
            # Launch the benchmark demo with default arguments but 3 participants and a non default registry URI to avoid collisions:
            |DemoDir|/SilKitDemoBenchmark --number-participants 3 --registry-uri silkit://localhost:8501

            # Launch the benchmark demo with positional arguments, a specified configuration file:
            |DemoDir|/SilKitDemoBenchmark 4 1 2 1 10 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml 

   *  -  Notes
      -  | This benchmark demo produces timings of a configurable simulation setup. <N> participants exchange <M> of <B> bytes per simulation step with a fixed simulation step size of 1ms and run for <S> seconds (virtual time).
         |
         | This simulation run is repeated <K> times and averages over all runs  are calculated. Results for average runtime, speedup (virtual time/runtime), throughput (data size/runtime), message rate (count/runtime) including the standard deviation are printed.
         |
         | The demo uses publish/subscribe controllers with the same topic for the message exchange, so each participant broadcasts the messages to all other participants. The configuration file ``DemoBenchmarkDomainSocketsOff.silkit.yaml`` can be used to disable domain socket usage for more realistic timings of TCP/IP traffic. With ``DemoBenchmarkTCPNagleOff.silkit.yaml``, Nagle's algorithm and domain sockets are switched off.
         |
         | The demo can be wrapped in helper scripts to run parameter scans, e.g., for performance analysis regarding different message sizes. See ``.\SilKit-Demos\Benchmark\msg-size-scaling\Readme.md`` and ``.\SilKit-Demos\Benchmark\performance-diff\Readme.md`` for further information.


Latency Demo
~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Latency Demo. Used for evaluating SIL Kit performance of publish/subscribe communication.
   *  -  Source location
      -  ./SilKit-Demos/Benchmark
   *  -  Requirements
      -  * :ref:`sil-kit-registry<sec:util-registry>`
   *  -  Positional parameters
      -  [messageCount]
           Sets the number of messages to be send in each simulation step.
         [messageSizeInBytes]
           Sets the message size.
         [registryURi] 
           The URI of the registry to start.
   *    - Optional parameters
        - --help
            Show the help message.
          --isReceiver
            This process is the receiving counterpart of the latency measurement. Default: false
          --registry-uri
            The URI of the registry to start. Default: silkit://localhost:8500
          --message-size
            Sets the message size. Default: 1000
          --message-count
            Sets the number of messages to be send in each simulation step. Default: 1000
          --configuration 
            Path and filename of the participant configuration YAML file. Default: empty
          --write-csv
            Path and filename of csv file with benchmark results. Default: empty
   *  -  Parameter Example
      -  .. parsed-literal:: 
            # Launch the two LatencyDemo instances with positional arguments and a specified configuration file:
            |DemoDir|/SilKitDemoLatency 100 1000
            |DemoDir|/SilKitDemoLatency 100 1000 --isReceiver

            # Launch the LatencyDemo with positional arguments and a specified configuration file:
            |DemoDir|/SilKitDemoLatency 100 1000 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml
   *  -  Notes
      -  | This latency demo produces timings of a configurable simulation setup. Two participants exchange <M> messages of <B> bytes without time synchronization.
         |
         | The demo uses publish/subscribe controllers performing a message roundtrip (ping-pong) to calculate latency and throughput timings.
         |
         | Note that the two participants must use the same parameters for valid measurement and one participant must use the ``--isReceiver`` flag.


.. _sec:util-netsim-demo:
         
Network Simulator Demo
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 205
   :stub-columns: 1

   *  - Abstract
      - Demo usage of the Network Simulation API
   *  - Source location
      - ./SilKit-Demos/NetworkSimulator
   *  - Requirements
      - * :ref:`sil-kit-registry<sec:util-registry>`
        * :ref:`sil-kit-system-controller<sec:util-system-controller>`
        * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
   *  - Parameters
      - <ParticipantConfiguration.yaml> 
          File name of the participant configuration to be used; 
          use ``DemoNetSim.silkit.yaml`` for an example configuration.
        <ParticipantName> 
          The name of the participant within the simulation.
        [RegistryUri] 
          The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
   *  - Parameter Example
      - .. parsed-literal:: 
           
           # Start the Network Simulator Demo with the given config and participant name
           |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
   *  - System Example
      - Interplay with CAN Demo:

        .. parsed-literal:: 

            # Registry (if not already running):
            |Registry|
            
            # Monitor (optional):
            |Monitor|

            # CAN Reader:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader

            # CAN Writer:
            |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter

            # System Controller:
            |SystemController| CanReader CanWriter NetworkSimulator

            # Network Simulator Demo:
            |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator

   *  -  Notes
      -  * The CAN Reader and Writer configure their controller on the network "CAN1", which is simulated by the network simulator demo.
         * In the simple bus logic of the network simulation demo (see ``Demos\NetworkSimulator\src\Can\MySimulatedCanController.cpp``), the acknowledgement (CanFrameTransmitEvent) is sent directly to the CAN Writer. The frame itself (CanFrameEvent) is sent with a delay of 2ms.