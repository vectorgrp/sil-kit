======================
Demos
======================

This document describes the usage of the demo projects that are
included with the Vector SIL Kit project and what their
expected output and or results are. All demo source code is located in
the Git repository in the folder Demos.

.. |UtilDir| replace:: build/Release
.. |DemoDir| replace:: build/Release
.. |SystemMonitor| replace::  |UtilDir|/sil-kit-system-monitor
.. |SystemController| replace::  |UtilDir|/sil-kit-system-controller

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
just build the  ``Demos`` CMake target::
    
    cmake --build . --target Demos

The individual demos are build as a dependency.

.. admonition:: Note
   
   The distributed Demos, as packaged by CPack, are preconfigured to build against 
   a copy of the SIL Kit binaries in ``../SilKit/`` .
   This can be overriden by providing your own ``SilKit`` CMake target library,
   before the demos are configured by cmake.
   Or by changing the ``find_package(SilKit ... PATHS path/to/SilKit)`` statement directly
   in the ``SilKit-Demos/CMakeLists.txt`` directory.


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
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
         * network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguration to be used; 
           use ``SilKitConfig_DemoCan.json`` for an example configuration.
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
            
            # Creates a CAN Writer Process with the default registry URI
            |DemoDir|/SilKitDemoCan Demos/Can/SilKitConfig_DemoCan.json CanWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # CAN Reader:
            |DemoDir|/SilKitDemoCan Demos/Can/SilKitConfig_DemoCan.json CanReader

            # CAN Writer:
            |DemoDir|/SilKitDemoCan Demos/Can/SilKitConfig_DemoCan.json CanWriter

            # System Controller:
            |SystemController| CanReader CanWriter 

        For unsynchronized execution:

        .. parsed-literal:: 

            # CAN Reader:
            |DemoDir|/SilKitDemoCan Demos/Can/SilKitConfig_DemoCan.json CanReader --async

            # CAN Writer:
            |DemoDir|/SilKitDemoCan Demos/Can/SilKitConfig_DemoCan.json CanWriter --async

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
      -  Demos/Ethernet
   *  -  Requirements
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>` (not needed for unsynchronized execution)
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
         * Network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``SilKitConfig_DemoEthernet.json`` for an example configuration.
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

            # Creates an Ethernet Writer Process with the default registry URI:
            |DemoDir|/SilKitDemoEthernet Demos/Ethernet/SilKitConfig_DemoEthernet.json EthernetWriter
   *  -  System Example
      - For synchronized execution:

        .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Ethernet Reader:
            |DemoDir|/SilKitDemoEthernet Demos/Ethernet/SilKitConfig_DemoEthernet.json EthernetReader

            # Ethernet Writer:
            |DemoDir|/SilKitDemoEthernet Demos/Ethernet/SilKitConfig_DemoEthernet.json EthernetWriter

            # System Controller:
            |SystemController| EthernetReader Ethernet Writer

        For unsynchronized execution:

        .. parsed-literal:: 

            # Ethernet Reader:
            |DemoDir|/SilKitDemoEthernet Demos/Ethernet/SilKitConfig_DemoEthernet.json EthernetReader --async

            # Ethernet Writer:
            |DemoDir|/SilKitDemoEthernet Demos/Ethernet/SilKitConfig_DemoEthernet.json EthernetWriter --async

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
      -  Demos/Lin
   *  -  Requirements
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>`
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
         * Network simulator (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``SilKitConfig_DemoLin.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``EthernetWriter`` or 
           ``EthernetReader``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).
   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a LIN Master Process with the default registry URI:
            |DemoDir|/SilKitDemoLin Demos/Lin/SilKitConfig_DemoLin.json LinMaster
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # LIN Master:
            |DemoDir|/SilKitDemoLin Demos/Lin/SilKitConfig_DemoLin.json LinMaster

            # LIN Slave:
            |DemoDir|/SilKitDemoLin Demos/Lin/SilKitConfig_DemoLin.json LinSlave

            # System Controller:
            |SystemController| LinSlave LinMaster
   *  -  Notes
      -  | \- The LIN demo can only run in a synchronized mode.
         | \- Both Master and Slave sleep for 500 milliseconds per simulation step to slow down execution.


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
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>`
         * Network simulator (mandatory)
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``SilKitConfig_DemoFlexRay.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Node0`` or 
           ``Node1``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a FlexRay Process for Node 0 with the default registry URI:
            |DemoDir|/SilKitDemoFlexray Demos/FlexRay/SilKitConfig_DemoFlexray.json Node0
   *  -  System Example
      -  .. parsed-literal:: 

            # Network simulator (assumed to be in PATH, necessary):
            NetworkSimulator Demos/FlexRay/NetworkSimulatorConfig.json

            # System Monitor (optional):
            |SystemMonitor|

            # Node 0:
            |DemoDir|/SilKitDemoFlexray Demos/FlexRay/SilKitConfig_DemoFlexray.json Node0

            # Node 1:
            |DemoDir|/SilKitDemoFlexray Demos/FlexRay/SilKitConfig_DemoFlexray.json Node1

            # System Controller:
            |SystemController| Node0 Node1 NetworkSimulator
   *  -  Notes
      -  Starting the FlexRay cycle takes quite some time, which is accurately modeled by the NetworkSimulator. 
         It takes somewhat between 50 and 100 ms until the first FlexRay messages are transmitted.


Publish & Subscribe Demo
~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Data Message Publish Subscribe Demo for a set of Publishers/Subscribers
   *  -  Source location
      -  Demos/PubSub
   *  -  Requirements
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>`
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``SilKitConfig_DemoPubSub.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Publisher`` or 
           ``Subscriber``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a combined publisher and subscriber with the default registry URI:
            |DemoDir|/SilKitDemoPubSub Demos/PubSub/SilKitConfig_DemoPubSub.json Publisher
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Publisher:
            |DemoDir|/SilKitDemoPubSub Demos/PubSub/SilKitConfig_DemoPubSub.json Publisher

            # Subscriber:
            |DemoDir|/SilKitDemoPubSub Demos/PubSub/SilKitConfig_DemoPubSub.json Subscriber

            # System Controller:
            |SystemController| Publisher Subscriber

   *  -  Notes
      -  The publisher and subscriber show how to serialize/deserialize different kinds of data with the built in serializer/deserializer.

RPC Demo
~~~~~~~~~~~~~~~~~~~~


.. list-table::
   :widths: 17 220
   :stub-columns: 1

   *  -  Abstract
      -  Remote Procedure Call Demo. The client triggers remote procedure calls on the server.
   *  -  Source location
      -  Demos/Rpc
   *  -  Requirements
      -  * :ref:`sil-kit-system-controller<sec:util-system-controller>`
         * :ref:`sil-kit-system-monitor<sec:util-system-monitor>` (optional)
   *  -  Parameters
      -  <ParticipantConfiguration.json|yaml> 
           File name of the ParticipantConfiguraiton to be used; 
           use ``SilKitConfig_DemoRpc.json`` for an example configuration.
         <ParticipantName> 
           The name of the participant within the simulation; must either be ``Server`` or 
           ``Client``.
         [RegistryUri] 
           The silkit:// URI of the registry to connect to; defaults to silkit://localhost:8500 (optional).

   *  -  Parameter Example
      -  .. parsed-literal:: 

            # Creates a Rpc-Server Process with the default registry URI:
            |DemoDir|/SilKitDemoRpc Demos/Rpc/SilKitConfig_DemoRpc.json Server
   *  -  System Example
      -  .. parsed-literal:: 

            # System Monitor (optional):
            |SystemMonitor|

            # Server:
            |DemoDir|/SilKitDemoRpc Demos/Rpc/SilKitConfig_DemoRpc.json Server

            # Client:
            |DemoDir|/SilKitDemoRpc Demos/Rpc/SilKitConfig_DemoRpc.json Client

            # System Controller:
            |SystemController| Server Client
   *  -  Notes
      -  ``Client`` participant has two RpcClients that call the ``Add100`` and ``Sort`` functions on the two RpcServers of the ``Server`` participant.
