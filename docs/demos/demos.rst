.. include:: /substitutions.rst

======================
Demos
======================

.. |UtilDir| replace:: ./SilKit/bin
.. |DemoDir| replace:: ./SilKit/bin
.. |Monitor| replace::  |UtilDir|/sil-kit-monitor
.. |Registry| replace::  |UtilDir|/sil-kit-registry
.. |SystemController| replace::  |UtilDir|/sil-kit-system-controller

.. |DemoAbstractCAN| replace:: 
  The `CanWriter` application sends CAN frames to the `CanReader` application including frame acknowledgment handling.
.. |DemoAbstractETH| replace:: 
  The `EthernetWriter` application sends Ethernet frames to the `EthernetReader` application including frame acknowledgment handling.
.. |DemoAbstractLIN| replace:: 
  A two-node LIN Setup with a `LinMaster` and a `LinSlave` application. 
  Includes a simple scheduling mechanism and demonstrates controller sleep / wakeup handling.
.. |DemoAbstractFlexRay| replace::
  A two-node FlexRay Setup with a full cluster and node parametrization. Includes POC Status handling, buffer updates and reconfiguration.
  This Demo requires a separate `Network Simulator` application to simulate the details of the FlexRay cluster, which is not included in the |ProductName|.
.. |DemoAbstractPubSub| replace:: 
  One application publishes GPS and temperature data, another application subscribes to these topics.
  Including (de-)serialization of the C++ structures into a transmittable format.
.. |DemoAbstractRPC| replace:: 
  The RPC server application provides two simple functions which are called by a RPC client application.
  Includes (de-)serialization of the function parameters.
.. |DemoAbstractBenchmark| replace:: 
  This demo sets up a simulation with various command line arguments for benchmarking purposes.
  A configurable amount of participants are spawned by a single process on different threads.
  The demo calculates averaged running times, throughput, speed-up and message rates for performance evaluation.
.. |DemoAbstractLatency| replace:: 
  A sender and a receiver application use the Publish/Subscribe services and measure the round trip time of the communication.
  This setup is useful to evaluate the performance of a |ProductName| setup running on different platforms.
  E.g., between a local host, a virtual machine, a remote network, etc.
.. |DemoAbstractNetSim| replace:: 
  Demonstrates the usage of the experimental |ProductName| NetworkSimulator API.
  A custom network simulation for CAN is set up, the network simulator application can be used together with the CAN demo.


This chapter describes the demo projects showcasing the core features of the |ProductName|.
Available demos are:

.. list-table:: 
   :widths: 40 220
   :header-rows: 1

   * - Demo
     - Abstract
   * - :ref:`sec:can-demo` 
     - |DemoAbstractCAN|
   * - :ref:`sec:eth-demo`  
     - |DemoAbstractETH|
   * - :ref:`sec:lin-demo`  
     - |DemoAbstractLIN|
   * - :ref:`sec:flexray-demo`  
     - |DemoAbstractFlexRay|
   * - :ref:`sec:pubsub-demo`  
     - |DemoAbstractPubSub|
   * - :ref:`sec:rpc-demo`  
     - |DemoAbstractRPC|
   * - :ref:`sec:benchmark-demo`  
     - |DemoAbstractBenchmark|
   * - :ref:`sec:latency-demo`  
     - |DemoAbstractLatency|
   * - :ref:`sec:netsim-demo`  
     - |DemoAbstractNetSim|


.. _sec:build-demos:

Building the Demos
~~~~~~~~~~~~~~~~~~

The |ProductName| Demos are not available as pre-built binaries and have to be compiled from source first.

* If you plan to use the demos as a starting point for further development, start by cloning the `SIL Kit git repository <https://github.com/vectorgrp/sil-kit>`_.
  Don't forget to call ``git submodule update --init --recursive`` after cloning the repository for the first time.
  Using the build instructions below will build the complete |ProductName| library including the Demos.
* If you just want to try out the demos, download a `SIL Kit release package <https://github.com/vectorgrp/sil-kit/releases>`_ which also includes all sources.
  Using the build instructions below will only build the |ProductName| Demos and use the precompiled library of the package.
* Building the Demos from the installation folder of the SIL Kit MSI Installer is strongly discouraged, as all involved tools require administrative privileges if the default installation directory is used.

There are several options to build the demos, all require the installation of a C++ compiler and CMake.

.. admonition:: Note

    If you don't have any experience with setting up C++ / CMake projects, we recommend using VS Code and a `SIL Kit release package <https://github.com/vectorgrp/sil-kit/releases>`_.
    VS Code is free to use, and the prerequisites can be quickly set up using the recommended VS Code extensions.

VS Code
-------

#. Install the VS Code extensions `C/C++` and `CMake` 
#. Open the folder with VS Code

   a. For the git repository: open the root folder of the repository
   b. For a |ProductName| package: open the `SilKit-Demos` folder
#. Opening the folder automatically starts the CMake configuration step.
   You can also manually call this step in the CMake extension page under `Project Status | Configure`.
#. In the CMake extension page, build the project under `Project Status | Build`
#. Locate the binaries

   a. For the git repository: The binaries reside in ``_build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

Visual Studio
-------------

#. Make sure Visual Studio is set up for C++ and CMake is installed
#. Open the folder with Visual Studio

   a. For the git repository: open the root folder of the repository
   b. For a |ProductName| package: open the `SilKit-Demos` folder
#. Opening the folder automatically starts the CMake configuration step. 
   You can also manually call this step under `Project | Configure Cache`.
#. Build the project with `Build | Build All`
#. Locate the binaries

   a. For the git repository: The binaries reside in ``_build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

Command Line
------------
 
#. Make sure a C++ compiler and CMake is installed
#. Open a terminal

   a. For the git repository: navigate to the root folder of the repository
   b. For a |ProductName| package: navigate to ``<package folder>/SilKit-Demos``
#. Run the following commands:
   
   * ``mkdir build``
   * ``cd build``
   * ``cmake ..``
   * ``cmake --build .``
#. Locate the binaries

   a. For the git repository: The binaries reside in ``build/<build config>/<build type, e.g. Debug, Release>/``
   b. For a |ProductName| package: The binaries reside in ``<package folder>/SilKit/bin``

Due to the large variety of terminals, different operating systems and availability of environment variables, the previous instruction can fail for several reasons.
Usually, the configuration step performed in  ``cmake ..`` will give useful hints of what went wrong.

Advanced CMake configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Build the Demos from a |ProductName| package against your own target library
   The distributed Demos, as packaged by CPack, are preconfigured to build against a copy of the |ProductName| binaries in ``../SilKit/``.
   This can be overridden by providing your own ``SilKit`` CMake target library, before the demos are configured by CMake.
   Or by changing the ``find_package(SilKit ... PATHS path/to/SilKit)`` statement directly in the ``./SilKit-Demos/CMakeLists.txt`` directory.


.. admonition:: Note

   In the following, the paths to run the demos are relative to the top level of the pre-built |ProductName| package.
   To adapt them to run from a git repository build, you have to change the relative path to the configuration file.
   E.g. use ``../../../Demos/Can/DemoCan.silkit.yaml`` instead of ``./SilKit-Demos/Can/DemoCan.silkit.yaml``.

   Further, the command line instructions use forward slashes for paths.
   When using Windows (e.g. Powershell), these have to be replaced with backward slashed.
   E.g., instead of ``./SilKit/bin/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader``, use ``.\SilKit\bin\SilKitDemoCan .\SilKit-Demos\Can\DemoCan.silkit.yaml CanReader``


.. _sec:can-demo:

CAN Demo
~~~~~~~~

Abstract
    |DemoAbstractCAN|
Source location
    ``./SilKit-Demos/Can``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>``
      File name of the participant configuration to be used; 
      use ``DemoCan.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>``
      The name of the participant within the simulation; must either be ``CanWriter`` or ``CanReader``.
    * ``[RegistryUri]``
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
    * ``[--async]`` 
      If async flag is set, the participant will join the simulation unsynchronized and it will not need
      the |SystemController| to start.
Parameter Example
    .. parsed-literal::

        # Creates a CAN Writer Process with the registry's default URI
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter

System Example
    Run the following commands in separate terminals:

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

    To run the demo without virtual time synchronization, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # CAN Reader:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader --async

        # CAN Writer:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter --async

Notes
    * The writer sends CAN frames at a fixed rate of one frame per simulation step (1ms).
    * Both reader and writer sleep for 1 second per simulation step to slow down the execution.


.. _sec:eth-demo:

Ethernet Demo
~~~~~~~~~~~~~

Abstract
    |DemoAbstractETH|
Source location
    ``./SilKit-Demos/Ethernet``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoEthernet.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation; must either be ``EthernetWriter`` or 
      ``EthernetReader``.
    * ``[RegistryUri]`` 
      The ``silkit://`` URI of the registry to connect to; defaults to ``silkit://localhost:8500`` (optional).
    * ``[--async]``  
      If async flag is set, the participant will join the simulation unsynchronized and it will not need
      the |SystemController| to start.
Parameter Example
    .. parsed-literal:: 

       # Creates an Ethernet Writer Process with the registry's default URI:
       |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetWriter
System Example
    Run the following commands in separate terminals:

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

    To run the demo without virtual time synchronization, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Ethernet Reader:
        |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetReader --async

        # Ethernet Writer:
        |DemoDir|/SilKitDemoEthernet ./SilKit-Demos/Ethernet/DemoEthernet.silkit.yaml EthernetWriter --async
Notes
    * The writer sends Ethernet frames at a fixed rate of one frame per simulation step (1ms).
    * Both reader and writer sleep for 1 second per simulation step to slow down execution.


.. _sec:lin-demo:

LIN Demo
~~~~~~~~

Abstract
    |DemoAbstractLIN|
Source location
    ``./SilKit-Demos/Lin``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoLin.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation; 
      must either be ``EthernetWriter`` or ``EthernetReader``.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
    * ``[--async]``  
      If async flag is set, the participant will join the simulation unsynchronized and it will not need the |SystemController| to start.
Parameter Example
    .. parsed-literal:: 

       # Creates a LIN Master Process with the registry's default URI:
       |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinMaster
System Example
    Run the following commands in separate terminals:

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

    To run the demo without virtual time synchronization, use the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # LIN Master:
       |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinMaster --async

       # LIN Slave:
       |DemoDir|/SilKitDemoLin ./SilKit-Demos/Lin/DemoLin.silkit.yaml LinSlave --async

Notes
    * Both Master and Slave sleep for 100ms per simulation step to slow down execution.

.. _sec:flexray-demo:

FlexRay Demo
~~~~~~~~~~~~

Abstract
    |DemoAbstractFlexRay|
Source location
    ``./SilKit-Demos/FlexRay``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
    * SIL Kit Network Simulator (mandatory)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoFlexRay.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation; must either be ``Node0`` or 
      ``Node1``.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
Parameter Example
    .. parsed-literal:: 

       # Creates a FlexRay Process for Node 0 with the registry's default URI:
       |DemoDir|/SilKitDemoFlexRay ./SilKit-Demos/FlexRay/DemoFlexRay.silkit.yaml Node0
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

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
Notes
    * Starting the FlexRay cycle takes quite some time, which is accurately modeled by the SIL Kit Network Simulator.
    * It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.

.. _sec:pubsub-demo:

Publish/Subscribe Demo
~~~~~~~~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractPubSub|
Source location
    ``./SilKit-Demos/PubSub``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoPubSub.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation; must either be ``Publisher`` or 
      ``Subscriber``.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
    * ``[--async]``  
      If async flag is set, the participant will join the simulation unsynchronized and it will not need
      the |SystemController| to start.
Parameter Example
    .. parsed-literal:: 
    
       # Creates a publisher with the registry's default URI:
       |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Publisher
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
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
    
    To run the demo without virtual time synchronization, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Publisher:
       |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Publisher --async
    
       # Subscriber:
       |DemoDir|/SilKitDemoPubSub ./SilKit-Demos/PubSub/DemoPubSub.silkit.yaml Subscriber --async
    
Notes
    * The publisher and subscriber show how to serialize/deserialize different kinds of data with the built-in :doc:`Data Serialization API</api/serdes>`.

.. _sec:rpc-demo:

RPC Demo
~~~~~~~~

Abstract
    |DemoAbstractRPC|
Source location
    ``./SilKit-Demos/Rpc``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoRpc.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation; must either be ``Server`` or 
      ``Client``.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
    * ``[--async]``  
      If async flag is set, the participant will join the simulation unsynchronized and it will not need
      the |SystemController| to start.
Parameter Example
    .. parsed-literal:: 
    
       # Creates an RPC server process with the registry's default URI:
       |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Server
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
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
    
    To run the demo without virtual time synchronization, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Server:
       |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Server --async
    
       # Client:
       |DemoDir|/SilKitDemoRpc ./SilKit-Demos/Rpc/DemoRpc.silkit.yaml Client --async
    
Notes
    * ``Client`` participant has two RPC clients which call the ``Add100`` and ``Sort`` functions on the ``Server`` participant's two RPC servers.


.. _sec:benchmark-demo:

Benchmark Demo
~~~~~~~~~~~~~~

Abstract
    |DemoAbstractBenchmark|
Source location
    ``./SilKit-Demos/Benchmark``
Requirements
    None (The demo starts its own instance of the registry and system controller).
Positional Parameters
    * ``[numberOfSimulationRuns]``
      Sets the number of simulation runs to perform.
    * ``[simulationDuration]``
      Sets the virtual simulation duration <S>.
    * ``[numberOfParticipants]``
      Sets the number of simulation participants <N>.
    * ``[messageCount]``
      Sets the number of messages <M> to be send in each simulation step.
    * ``[messageSizeInBytes]``
      Sets the message size <B>.
    * ``[registryURi]`` 
      The URI of the registry to start.
Optional Parameters
    * ``--help``
      Show the help message.
    * ``--registry-uri``
      The URI of the registry to start. Default: silkit://localhost:8500
    * ``--message-size``
      Sets the message size <B> in bytes. Default: 1000
    * ``--message-count``
      Sets the number of messages <M> to be send in each simulation step. Default: 50
    * ``--number-participants``
      Sets the number of simulation participants <N>. Default: 2
    * ``--number-simulation-runs``
      Sets the number of simulation runs to perform. Default: 4
    * ``--simulation-duration``
      Sets the simulation duration <S> (virtual time). Default: 1s
    * ``--configuration``
      Path and filename of the participant configuration YAML file. Default: empty
    * ``--write-csv``
      Path and filename of CSV file with benchmark results. Default: empty
System Examples
    * Launch the benchmark demo with default arguments but 3 participants:

      .. parsed-literal::
      
         |DemoDir|/SilKitDemoBenchmark --number-participants 3
    * Launch the benchmark demo with positional arguments and a configuration file that enforces TCP communication:

      .. parsed-literal:: 

         |DemoDir|/SilKitDemoBenchmark 4 1 2 1 10 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml 
Notes
    * This benchmark demo produces timings of a configurable simulation setup.
      <N> participants exchange <M> messages of <B> bytes per simulation step with a fixed simulation step size of 1ms and run for <S> seconds (virtual time).
    * This simulation run is repeated <K> times and averages over all runs are calculated. 
      Results for average runtime, speedup (virtual time/runtime), throughput (data size/runtime), message rate (count/runtime) including the standard deviation are printed.
    * The demo uses publish/subscribe controllers with the same topic for the message exchange, so each participant broadcasts the messages to all other participants. 
      The configuration file ``DemoBenchmarkDomainSocketsOff.silkit.yaml`` can be used to disable domain socket usage for more realistic timings of TCP/IP traffic. With ``DemoBenchmarkTCPNagleOff.silkit.yaml``, Nagle's algorithm and domain sockets are switched off.
    * The demo can be wrapped in helper scripts to run parameter scans, e.g., for performance analysis regarding different message sizes. 
      See ``.\SilKit-Demos\Benchmark\msg-size-scaling\Readme.md`` and ``.\SilKit-Demos\Benchmark\performance-diff\Readme.md`` for further information.
         
         
.. _sec:latency-demo:

Latency Demo
~~~~~~~~~~~~

Abstract
    |DemoAbstractLatency|
Source location
    ``./SilKit-Demos/Benchmark``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
Positional Parameters
    * ``[messageCount]``
      Sets the number of messages to be send in each simulation step.
    * ``[messageSizeInBytes]``
      Sets the message size.
    * ``[registryURi]`` 
      The URI of the registry to start.
Optional Parameters
    * ``--help``
      Show the help message.
    * ``--isReceiver``
      This process is the receiving counterpart of the latency measurement. Default: false
    * ``--registry-uri``
      The URI of the registry to start. Default: silkit://localhost:8500
    * ``--message-size``
      Sets the message size. Default: 1000
    * ``--message-count``
      Sets the number of messages to be send in each simulation step. Default: 1000
    * ``--configuration``
      Path and filename of the participant configuration YAML file. Default: empty
    * ``--write-csv``
      Path and filename of csv file with benchmark results. Default: empty
System Examples
    * Launch the two LatencyDemo instances with positional arguments in separate terminals:
      .. parsed-literal:: 

         |DemoDir|/SilKitDemoLatency 100 1000
         |DemoDir|/SilKitDemoLatency 100 1000 --isReceiver

    * Launch the LatencyDemo instances with a configuration file that enforces TCP communication:
      .. parsed-literal:: 

         |DemoDir|/SilKitDemoLatency 100 1000 --configuration ./SilKit-Demos/Benchmark/DemoBenchmarkDomainSocketsOff.silkit.yaml
         |DemoDir|/SilKitDemoLatency 100 1000 --isReceiver
Notes
    * This latency demo produces timings of a configurable simulation setup. 
      Two participants exchange <M> messages of <B> bytes without time synchronization.
    * The demo uses publish/subscribe controllers performing a message round trip (ping-pong) to calculate latency and throughput timings.
    * Note that the two participants must use the same parameters for valid measurement and one participant must use the ``--isReceiver`` flag.


.. _sec:netsim-demo:
         
Network Simulator Demo
~~~~~~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractNetSim|
Source location
    ``./SilKit-Demos/NetworkSimulator``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``<ParticipantConfiguration.yaml>`` 
      File name of the participant configuration to be used; 
      use ``DemoNetSim.silkit.yaml`` for an example configuration.
    * ``<ParticipantName>`` 
      The name of the participant within the simulation.
    * ``[RegistryUri]`` 
      The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
Parameter Example
    .. parsed-literal:: 
       
       # Start the Network Simulator Demo with the given configuration file and participant name
       |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
System Example
    Interplay with the CAN Demo:

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
    
Notes
    * The CAN Reader and Writer configure their controller on the network "CAN1", which is simulated by the network simulator demo.
    * In the simple bus logic of the network simulation demo (see ``Demos\NetworkSimulator\src\Can\MySimulatedCanController.cpp``), the acknowledgment (CanFrameTransmitEvent) is sent directly to the CAN Writer. 
      The frame itself (CanFrameEvent) is sent with a delay of 2ms.

