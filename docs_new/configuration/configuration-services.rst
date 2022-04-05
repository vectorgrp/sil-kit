==============================================================
Service Configuration (CAN, LIN, FlexRay, Ethernet, Data, RPC)
==============================================================

.. contents:: :local:
   :depth: 3

Overview
========================================
All services of the Vector Integration Bus can be configured within the participant configuration.
The following sections describe how each service can be configured.
The configuration of the single services is fully optional.
       
.. _sec:cfg-participant-can:

CanControllers
=============================

.. code-block:: javascript
    
  "CanControllers": [
      {
          "Name": "CAN1",
          "Network": "CAN1"
      }
  ]

.. list-table:: CanController Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the CAN Controller
   * - Network
     - The name of the CAN Network to connect to (optional)
   * - UseTraceSinks
     - A list of :ref:`trace sinks<sec:cfg-participant-tracesink>` to be used by
       this controller. Trace sinks are referred to by their name and can be used
       by multiple controllers. (optional)
   * - Replay
     - A :ref:`replay configuration <sec:cfg-participant-replay>` to be used
       by this controller. (optional)

.. _sec:cfg-participant-lin:

LinControllers
=============================

.. code-block:: javascript
    
  "LinControllers": [
      {
          "Name": "Lin1",
          "Network": "Lin1"
      }
  ]

.. list-table:: LinController Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the LIN Controller
   * - Network
     - The name of the LIN Network to connect to (optional)
   * - UseTraceSinks
     - A list of :ref:`trace sinks<sec:cfg-participant-tracesink>` to be used by
       this controller. Trace sinks are referred to by their name and can be used
       by multiple controllers. (optional)
   * - Replay
     - A :ref:`replay configuration <sec:cfg-participant-replay>` to be used
       by this controller. (optional)


.. _sec:cfg-participant-ethernet:

EthernetControllers
=============================

.. code-block:: javascript
    
  "EthernetControllers": [
      {
          "Name": "ETH1",
          "Network": "Ethernet1"
          "MacAddress": "00:08:15:ab:cd:ef"
      },
      {
          "Name": "ETH2",
          "MacAddress": "00:08:15:ab:cd:f0",
          "UseTraceSinks": ["MyPcapSink"]
      }
  ]


.. list-table:: Ethernet Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the Ethernet Controller
   * - MacAddress
     - The colon-separated Ethernet MAC address. 
   * - Network
     - The name of the Ethernet Network to connect to (optional)
   * - UseTraceSinks
     - A list of :ref:`trace sinks<sec:cfg-participant-tracesink>` to be used by
       this controller. Trace sinks are referred to by their name and can be used
       by multiple controllers. (optional)
   * - (PcapFile) 
     - (deprecated, use *UseTraceSinks* instead)
   * - (PcapPipe)
     - (deprecated, use *UseTraceSinks* instead)
   * - Replay
     - A :ref:`replay configuration <sec:cfg-participant-replay>` to be used
       by this controller. (optional)


.. _sec:cfg-participant-flexray:

FlexRayControllers
=============================

.. code-block:: javascript
    
  "FlexRayControllers": [
      {
          "Name": "FlexRay1",
          "Network": "PowerTrainCluster1",
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
     - Allows to configure cluster specific settings. (optional)
   * - NodeParameters
     - Allows to configure node specific settings. (optional)
   * - TxBufferConfigurations
     - Allows to configure TXBuffers by specifying "channels" (A, B, AB, None), 
       "slotId", "offset", "repetition", "PPindicator", "headerCrc" 
       and "transmissionMode" (SingleShot, Continuous). (optional)
   * - UseTraceSinks
     - A list of :ref:`trace sinks<sec:cfg-participant-tracesink>` to be used by
       this controller. Trace sinks are referred to by their name and can be used
       by multiple controllers. (optional)
   * - Replay
     - A :ref:`replay configuration <sec:cfg-participant-replay>` to be used
       by this controller. (optional)

.. _sec:cfg-participant-data-publishers:

DataPublishers
=============================

.. code-block:: javascript
    
  "DataPublishers": []


.. list-table:: DataPublisher Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - (No fields yet)
     - 

.. _sec:cfg-participant-data-subscribers:

DataSubscribers
=============================

.. code-block:: javascript
    
  "DataSubscribers": []


.. list-table:: DataSubscriber Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - (No fields yet)
     - 

.. _sec:cfg-participant-rpc-servers:

RpcServers
=============================


.. code-block:: javascript
    
  "RpcServers": []


.. list-table:: RPC Server Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - (No fields yet)
     - 


.. _sec:cfg-participant-rpc-clients:

RpcClients
=============================

.. code-block:: javascript
    
  "DataPublishers": []


.. list-table:: RPC Clients Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - (No fields yet)
     - 
