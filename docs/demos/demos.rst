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
  The `CanWriter` participant sends Can frames to the `CanReader` participant including frame acknowledgment handling.
.. |DemoAbstractETH| replace:: 
  The `EthernetWriter` participant sends Ethernet frames to the `EthernetReader` participant including frame acknowledgment handling.
.. |DemoAbstractLIN| replace:: 
  A two-node Lin Setup with a `LinMaster` and a `LinSlave`. 
  Includes a simple scheduling mechanism and demonstrates controller sleep / wakeup handling.
.. |DemoAbstractFlexRay| replace::
  A two-node FlexRay Setup with a full cluster and node parametrization. Includes POC Status handling, buffer updates and reconfiguration.
  This Demo requires a separate `Network Simulator` application to simulate the details of the FlexRay cluster, which is not included in the |ProductName|.
.. |DemoAbstractPubSub| replace:: 
  One participant publishes GPS and temperature data, another participant subscribes to these topics.
  Including (de-)serialization of the C++ structures into a transmittable format.
.. |DemoAbstractRPC| replace:: 
  The Rpc server participant provides two simple functions which are called by a Rpc client participant.
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
  A custom network simulation for Can is set up, the network simulator application can be used together with the Can demo.


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

From command line
-----------------
 
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


.. _sec:common-cla:

Demo command line arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some general settings for |ProductName| participants are available through a common set of command line arguments. 
The demos for Can, Ethernet, Lin, Flexray, PubSub and Rpc all share the following arguments:

.. code-block:: console

    -h, --help                   | Get this help.
    -n, --name <name>            | The participant name used to take part in the simulation.
                                   Defaults to '<set by the individual demo>'.
    -u, --registry-uri <uri>     | The registry URI to connect to.
                                   Defaults to 'silkit://localhost:8500'.
    -l, --log <level>            | Log to stdout with level:
                                   'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'.
                                   Defaults to 'info'.
                                   Cannot be used together with '--config'.
    -c, --config <filePath>      | Path to the Participant configuration YAML or JSON file.
                                   Cannot be used together with '--log'.
                                   Will always run as fast as possible.
    -a, --async                  | Run without time synchronization mode.
                                   Cannot be used together with '--sim-step-duration'.
    -A, --autonomous             | Start the simulation autonomously.
                                   Without this flag, a coordinated start is performed
                                   which requires the SIL Kit System Controller.
    -d, --sim-step-duration <us> | The duration of a simulation step in microseconds.
                                   Defaults to <set by the individual demo>us.
                                   Cannot be used together with '--async'.
    -f, --fast                   | Run the simulation as fast as possible.
                                   By default, the execution is slowed down to two work cycles per second.
                                   Cannot be used together with '--config'.

The default behavior of these options is:

* Participant name is set by the individual demo executable (e.g. ``CanWriter`` for ``SilKitDemoCanWriter``).
* Default registry-uri ``silkit://localhost:8500``.
* Logging to Stdout with Level Info.
* No participant configuration file in use.
* Virtual time synchronization enabled.
* Coordinated start (requires a ``sil-kit-system-controller``).
* Simulation step duration set by the individual demo executable.
* Slowed down execution to two work cycles per second.

Useful execution modes that are made accessible by the general options:

* Spawn multiple demo participants by using collision free participant names, e.g. add a second Can reader with  ``--name CanReader1``.
* Run without time synchronization and start coordination: ``--async --autonomous``, or short ``-aA``.
  This allows to start/stop the participants individually without requiring a ``sil-kit-system-controller``.
* Join a already running simulation with time synchronization: ``--autonomous`` (without ``--async``).
* Perform a coordinated start without time synchronization: ``--async`` (without ``--autonomous``).
  This requires a ``sil-kit-system-controller``.

Some demos extend these options by the following command line arguments:

* For the bus demos (Can, Ethernet, Lin, Flexray): ``--network`` to override the default bus network name.
* For Can and Ethernet: ``--hex`` to print payloads in hexadecimal format.

.. _sec:sample-configs:

Sample participant configurations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following sample participant configurations are available:

 * ``FileLog_Trace.silkit.yaml``: Log to a file with Level Trace.
 * ``FileLog_Trace_FromRemotes.silkit.yaml``: Log to file with Level Trace from participants that use a Sink of Type Remote.
 * ``Stdout_Info.silkit.yaml``: Log to Stdout with Level Info.
 * ``Trace_ToRemote.silkit.yaml``: Log to Remote with Level Info. Another participant must specify LogFromRemotes True.

.. _sec:can-demo:

Can Demo
~~~~~~~~

Abstract
    |DemoAbstractCAN|
Executables
    * ``SilKitDemoCanReader``
    * ``SilKitDemoCanWriter``
Source location
    ``./SilKit-Demos/Can``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Can network to use. 
      Defaults to "CAN1".
    * ``--hex``
      Print the Can payloads in hexadecimal format.
      Otherwise, the payloads are interpreted as strings.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal::

        # Registry (if not already running):
        |Registry|
            
        # Monitor (optional):
        |Monitor|

        # Can Reader:
        |DemoDir|/SilKitDemoCanReader

        # Can Writer:
        |DemoDir|/SilKitDemoCanWriter

        # System Controller:
        |SystemController| CanReader CanWriter 

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Can Reader:
        |DemoDir|/SilKitDemoCanReader --async --autonomous

        # Can Writer:
        |DemoDir|/SilKitDemoCanWriter --async --autonomous

.. _sec:eth-demo:

Ethernet Demo
~~~~~~~~~~~~~

Abstract
    |DemoAbstractETH|
Executables
    * ``SilKitDemoEthernetReader``
    * ``SilKitDemoEthernetWriter``
Source location
    ``./SilKit-Demos/Ethernet``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Ethernet network to use. 
      Defaults to "Eth1".
    * ``--hex``
      Print the Ethernet payloads in hexadecimal format.
      Otherwise, the payloads are interpreted as strings.
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Ethernet Reader:
        |DemoDir|/SilKitDemoEthernetReader

        # Ethernet Writer:
        |DemoDir|/SilKitDemoEthernetWriter

        # System Controller:
        |SystemController| EthernetReader EthernetWriter

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

        # Registry (if not already running):
        |Registry|

        # Ethernet Reader:
        |DemoDir|/SilKitDemoEthernetReader --async --autonomous

        # Ethernet Writer:
        |DemoDir|/SilKitDemoEthernetWriter --async --autonomous

.. _sec:lin-demo:

Lin Demo
~~~~~~~~

Abstract
    |DemoAbstractLIN|
Executables
    * ``SilKitDemoLinMaster``
    * ``SilKitDemoLinSlave``
Source location
    ``./SilKit-Demos/Lin``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
    * SIL Kit Network Simulator (optional)
Parameters
    * ``--network <name>``
      Name of the Lin network to use. 
      Defaults to "LIN1".
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # Monitor (optional):
       |Monitor|

       # Lin Master:
       |DemoDir|/SilKitDemoLinMaster

       # Lin Slave:
       |DemoDir|/SilKitDemoLinSlave

       # System Controller:
       |SystemController| LinSlave LinMaster

    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:

    .. parsed-literal:: 

       # Registry (if not already running):
       |Registry|

       # Lin Master:
       |DemoDir|/SilKitDemoLinMaster --async --autonomous

       # Lin Slave:
       |DemoDir|/SilKitDemoLinSlave --async --autonomous

.. _sec:flexray-demo:

FlexRay Demo
~~~~~~~~~~~~

Abstract
    |DemoAbstractFlexRay|
Executables
    * ``SilKitDemoFlexrayNode0``
    * ``SilKitDemoFlexrayNode1``
Source location
    ``./SilKit-Demos/FlexRay``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>`
    * SIL Kit Network Simulator (mandatory)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * ``--network <name>``
      Name of the FlexRay network to use. 
      Defaults to "PowerTrain1".
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
       |DemoDir|/SilKitDemoFlexrayNode0

       # Node 1:
       |DemoDir|/SilKitDemoFlexrayNode1

       # System Controller:
       |SystemController| Node0 Node1 NetworkSimulator
Notes
    * The FlexRay demo requires the usage of the SIL Kit Network Simulator and virtual time synchronization.
    * It takes about 65ms (virtual time) until the starting the FlexRay cycle has started and the first FlexRay messages are transmitted.

.. _sec:pubsub-demo:

Publish/Subscribe Demo
~~~~~~~~~~~~~~~~~~~~~~~~

Abstract
    |DemoAbstractPubSub|
Executables
    * ``SilKitDemoPublisher``
    * ``SilKitDemoSubscriber``
Source location
    ``./SilKit-Demos/PubSub``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * No demo specific command line arguments
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
       # Registry (if not already running):
       |Registry|
    
       # Monitor (optional):
       |Monitor|
    
       # Publisher:
       |DemoDir|/SilKitDemoPublisher 
    
       # Subscriber:
       |DemoDir|/SilKitDemoSubscriber
    
       # System Controller:
       |SystemController| Publisher Subscriber
    
    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Publisher:
       |DemoDir|/SilKitDemoPublisher --async --autonomous
    
       # Subscriber:
       |DemoDir|/SilKitDemoSubscriber --async --autonomous

.. _sec:rpc-demo:

Rpc Demo
~~~~~~~~

Abstract
    |DemoAbstractRPC|
Executables
    * ``SilKitDemoRpcClient``
    * ``SilKitDemoRpcServer``
Source location
    ``./SilKit-Demos/Rpc``
Requirements
    * :ref:`sil-kit-registry<sec:util-registry>`
    * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
    * :ref:`sil-kit-monitor<sec:util-monitor>` (optional)
Parameters
    * No demo specific command line arguments
System Example
    Run the following commands in separate terminals:

    .. parsed-literal:: 
    
       # Registry (if not already running):
       |Registry|
    
       # Monitor (optional):
       |Monitor|
    
       # Server:
       |DemoDir|/SilKitDemoRpcServer
    
       # Client:
       |DemoDir|/SilKitDemoRpcClient
    
       # System Controller:
       |SystemController| RpcServer RpcClient
    
    To run the demo without virtual time synchronization and start coordination, use the following commands in separate terminals:
    
    .. parsed-literal::
    
       # Registry (if not already running):
       |Registry|
    
       # Server:
       |DemoDir|/SilKitDemoRpcServer --async --autonomous
    
       # Client:
       |DemoDir|/SilKitDemoRpcClient --async --autonomous
    
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
    Interplay with the Can Demo:

    .. parsed-literal:: 
    
        # Registry (if not already running):
        |Registry|
        
        # Monitor (optional):
        |Monitor|
    
        # Can Reader:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanReader
    
        # Can Writer:
        |DemoDir|/SilKitDemoCan ./SilKit-Demos/Can/DemoCan.silkit.yaml CanWriter
    
        # System Controller:
        |SystemController| CanReader CanWriter NetworkSimulator
    
        # Network Simulator Demo:
        |DemoDir|/SilKitDemoNetSim ./SilKit-Demos/NetworkSimulator/DemoNetSim.silkit.silkit.yaml NetworkSimulator
    
Notes
    * The Can Reader and Writer configure their controller on the network "CAN1", which is simulated by the network simulator demo.
    * In the simple bus logic of the network simulation demo (see ``Demos\NetworkSimulator\src\Can\MySimulatedCanController.cpp``), the acknowledgment (CanFrameTransmitEvent) is sent directly to the Can Writer. 
      The frame itself (CanFrameEvent) is sent with a delay of 2ms.


Demo implementation details
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The communication demos Can, Ethernet, Lin, Flexray, PubSub and Rpc share a base implementation that provides general |ProductName| features.
This allows to separate the demo specific use case (e.g. how to use Ethernet with the |ProductName|) from general features (e.g. setup the |ProductName| lifecycle).
This base implementation is located in ``Demos\include\ApplicationBase.hpp``.

Further, the participant pairs of the demos share some common behavior like printing frames, status handling or small helper classes.
This is located in the header ``<DemoName>Common.hpp`` of the respective demo folders.

To constructed your own demo, use the following template:

.. code-block:: c++

    #include "ApplicationBase.hpp"

    // Inherit from ApplicationBase that provides common SIL Kit features
    class MyDemoParticipant: public ApplicationBase
    {
    public:
        // Inherit constructors
        using ApplicationBase::ApplicationBase;

    private:
        
        // Member variables like SIL Kit controller pointers, Demo state, etc
        std::string _myOption;
        bool _myFlag;

        // The following overrides are invoked in the right order by the ApplicationBase
        // This enables:
        // - General and demo specific command line arguments
        // - Setup of the SIL Kit lifecycle
        // - Controller creation and initialization
        // - SimulationStepHandler vs. thread based execution (--async)
        // - Signal handling to CTRL-C at any time
        // - Basic logging

        // Extend the command line argument list
        void AddCommandLineArgs() override
        {
            GetCommandLineParser()->Add<CommandlineParser::Option>(
                "myOption", "o", "DefaultValue", "-o, --myOption <value>",
                std::vector<std::string>{"Description Line 1.", "Description Line 2"});

            GetCommandLineParser()->Add<CommandlineParser::Flag>(
                "myFlag", "f", "-f, --myFlag",
                std::vector<std::string>{"Description Line 1.", "Description Line 2"});
        }

        // Evaluate the command line argument list
        void EvaluateCommandLineArgs() override
        {
            _myOption = GetCommandLineParser()->Get<CommandlineParser::Option>("myOption").Value();
            _myFlag = GetCommandLineParser()->Get<CommandlineParser::Flag>("myFlag").Value();
        }

        // Create all SIL Kit controllers here
        void CreateControllers() override
        {
            // All SIL Kit features can be accessed via GetParticipant()
            // _myController = GetParticipant()->CreateXYZController(...);
        }

        // Controller initialization goes here
        void InitControllers() override
        {
        }
    
        // Called in each simulation step when running with time synchronization
        void DoWorkSync(std::chrono::nanoseconds now) override
        {
            // _myController->Send(...)   
        }

        // Called in a worker thread when running without time synchronization
        void DoWorkAsync() override
        {
            // _myController->Send(...)   
        }
    };

    int main(int argc, char** argv)
    {
        Arguments args;
        args.participantName = "MyDemoParticipant"; // Always specify a meaningful default participant name 

        MyDemoParticipant app{args};

        // This will trigger AddCommandLineArgs() and EvaluateCommandLineArgs()
        // Optionally, a set of excluded default command line arguments can be specified
        app.SetupCommandLineArgs(argc, argv, "Description for the command line help");
    
        // This will trigger CreateControllers(), InitControllers() and then cyclically DoWorkSync() or DoWorkAsync()
        return app.Run();
    }
