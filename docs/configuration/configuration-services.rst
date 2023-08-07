===============================================================
Communication Services (CAN, LIN, FlexRay, Ethernet, Data, RPC)
===============================================================

.. contents:: :local:
   :depth: 3

Overview
========================================
All services of the Vector SIL Kit can be configured within the participant configuration.
The following sections describe how each service can be configured.
The configuration of the single services is fully optional.
       

.. _sec:cfg-participant-can:

CanControllers
=============================

.. code-block:: yaml
    
    CanControllers:
    - Name: CAN1
      Network: CAN1


.. list-table:: CanController Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the CAN Controller
   * - Network
     - The name of the CAN Network to connect to (optional)


.. _sec:cfg-participant-lin:

LinControllers
=============================

.. code-block:: yaml
    
    LinControllers:
    - Name: Lin1
      Network: Lin1


.. list-table:: LinController Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the LIN Controller
   * - Network
     - The name of the LIN Network to connect to (optional)


.. _sec:cfg-participant-ethernet:

EthernetControllers
=============================

.. code-block:: yaml
    
     EthernetControllers:
     - Name: ETH1
       Network: Ethernet1



.. list-table:: Ethernet Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the Ethernet Controller
   * - Network
     - The name of the Ethernet Network to connect to (optional)
   * - UseTraceSinks
     - **Experimental**: Optional list of names of trace sinks, as defined in the :ref:`Tracing<sec:cfg-participant-tracing>` configuration.
   * - Replay
     - **Experimental**: The optional replay configuration, as described in :ref:`Replay<sec:cfg-participant-replay>`.


.. _sec:cfg-participant-flexray:

FlexrayControllers
==================

.. code-block:: yaml
    
    FlexrayControllers:
    - Name: FlexRay1
      Network: PowerTrainCluster1
      ClusterParameters:
        gColdstartAttempts: 8
      NodeParameters:
        pChannels: AB


.. list-table:: FlexRay Controller Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the FlexRay Controller
   * - ClusterParameters
     - Allows to configure cluster specific settings. See ``ParticipantConfiguration.schema.json`` for a full set of parameters. (optional)
   * - NodeParameters
     - Allows to configure node specific settings. See ``ParticipantConfiguration.schema.json`` for a full set of parameters. (optional)
   * - TxBufferConfigurations
     - Allows to configure TXBuffers by specifying "channels" (``A``, ``B``, ``AB``, ``None``), 
       "slotId", "offset", "repetition", "PPindicator", "headerCrc" 
       and "transmissionMode" (``SingleShot``, ``Continuous``). See ``ParticipantConfiguration.schema.json`` for a full set of parameters. (optional)


.. _sec:cfg-participant-data-publishers:

DataPublishers
=============================

.. code-block:: yaml
    
  DataPublishers: 
  - Name: DataPublisher1
    Topic: SomeTopic1


.. list-table:: DataPublisher Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the data publisher.
   * - Topic
     - The topic on which the data publisher publishes its information. (optional)


.. _sec:cfg-participant-data-subscribers:

DataSubscribers
=============================

.. code-block:: yaml
    
  DataSubscribers: 
  - Name: DataSubscriber1
    Topic: SomeTopic1


.. list-table:: DataSubscriber Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the data subscriber.
   * - Topic
     - The topic on which the data subscriber publishes its information. (optional)


.. _sec:cfg-participant-rpc-servers:

RpcServers
=============================


.. code-block:: yaml
    
  RpcServers:
  - Name: RpcServer1
    FunctionName: SomeFunction1


.. list-table:: RPC Server Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the RPC server.
   * - FunctionName
     - The function name on which the RPC server offers its service. (optional)


.. _sec:cfg-participant-rpc-clients:

RpcClients
=============================

.. code-block:: yaml
    
  RpcClients: 
  - Name: RpcClient1
    FunctionName: SomeFunction1


.. list-table:: RPC Clients Configuration
   :widths: 15 85
   :header-rows: 1

   * - Property Name
     - Description
   * - Name
     - The name of the RPC client.
   * - FunctionName
     - The function name to which the RPC client wants to connect to. (optional)
