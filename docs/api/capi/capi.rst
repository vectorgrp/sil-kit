.. _sec:capi:

===================
Experimental C API
===================

.. contents::
   :local:
   :depth: 3


.. highlight:: c

VIB Entry Point and API Organization
====================================

The main entry point of the C API is the function to obtain a ib_SimulationParticipant*::

    ib_SimulationParticipant* participant = NULL;
    const char* jsonString = "{ ... }";
    const char* participantName = "CanWriter";
    const char* domainId = "1";
    ib_ReturnCode result = ib_SimulationParticipant_Create(&participant, jsonString,
                                                            participantName, ib_False, domainId);

All further services of the C API of the IntegrationBus are requested through this SimulationParticipant.

Entities obtained through the SimulationParticipant must not be destroyed/deleted by the user of the API.
All entities, that are provided through the API expect for the ib_SimulationParticipant are destroyed through
the internals of the Integration Bus implementation.

After creation of a SimulationParticipant it must be ensured that eventually ib_SimulationParticipant_Destroy is called
with the corresponding pointer to the ib_SimulationParticipant entity.


API and Data Type Reference
===========================

General API
-----------
.. doxygenfunction:: ib_ReturnCodeToString
.. doxygenfunction:: ib_GetLastErrorString

SimulationParticipant API
-------------------------

.. doxygenfunction:: ib_SimulationParticipant_Create
.. doxygenfunction:: ib_SimulationParticipant_Destroy

Most creator functions for other objects (such as bus controllers) require an ib_SimulationParticipant, 
which is the factory object, as input parameter.

Can API
-------
.. doxygenfunction:: ib_Can_Controller_Create
.. doxygenfunction:: ib_Can_Controller_Start
.. doxygenfunction:: ib_Can_Controller_Stop
.. doxygenfunction:: ib_Can_Controller_Reset
.. doxygenfunction:: ib_Can_Controller_Sleep
.. doxygenfunction:: ib_Can_Controller_SendFrame
.. doxygenfunction:: ib_Can_Controller_SetBaudRate
.. doxygenfunction:: ib_Can_Controller_RegisterTransmitStatusHandler
.. doxygenfunction:: ib_Can_Controller_RegisterReceiveMessageHandler
.. doxygenfunction:: ib_Can_Controller_RegisterStateChangedHandler
.. doxygenfunction:: ib_Can_Controller_RegisterErrorStateChangedHandler

Ethernet API
------------
The Ethernet API consists of two main parts:

# The Ethernet controller
# The Ethernet frame

Ethernet Controller
~~~~~~~~~~~~~~~~~~~

The Ethernet controller interacts with the corresponding Ethernet bus and send Ethernet frames.
The Ethernet frames are the single messages/frames, that are transmitted over the Ethernet bus.

.. doxygenfunction:: ib_Ethernet_Controller_Create
.. doxygenfunction:: ib_Ethernet_Controller_Activate
.. doxygenfunction:: ib_Ethernet_Controller_Deactivate
.. doxygenfunction:: ib_Ethernet_Controller_RegisterReceiveMessageHandler
.. doxygenfunction:: ib_Ethernet_Controller_RegisterFrameAckHandler
.. doxygenfunction:: ib_Ethernet_Controller_RegisterStateChangedHandler
.. doxygenfunction:: ib_Ethernet_Controller_RegisterBitRateChangedHandler
.. doxygenfunction:: ib_Ethernet_Controller_SendFrame

Ethernet Frame
~~~~~~~~~~~~~~

The ib_Ethernet_Frame corresponds to an ethernet raw frame.
A basic frame consists of the destination mac, the source mac, the ethertype and a payload.
The union type within the ib_Ethernet_Frame helps when manual construction of a frame is necessary (frameHeader, frameHeaderVlanTagged).

.. note:: For an example of manual frame construction one can refer to the C Ethernet demo.

Data API
--------
The Data API provides data publish and subscribe functionalities to the Integration Bus. 
It consists of DataPublishers and DataSubscribers.

DataPublishers
~~~~~~~~~~~~~~
.. doxygenfunction:: ib_Data_Publisher_Create
.. doxygenfunction:: ib_Data_Publisher_Publish

DataSubscribers
~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_Data_Subscriber_Create
.. doxygenfunction:: ib_Data_Subscriber_SetDefaultReceiveDataHandler
.. doxygenfunction:: ib_Data_Subscriber_RegisterSpecificDataHandler

Handlers
~~~~~~~~
The DataSubscriber is created with a handler for data reception and a handler
for notification about new sources:

.. doxygentypedef:: ib_Data_Handler_t
.. doxygentypedef:: ib_Data_NewDataSourceHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Data_ExchangeFormat
   :members:

Rpc API
-------
The Rpc API provides client/server based Rpc functionality. 
It consists of RpcClients and RpcServers and a method to discover remote RpcServers.

RpcClients
~~~~~~~~~~
.. doxygenfunction:: ib_Rpc_Client_Create
.. doxygenfunction:: ib_Rpc_Client_Call

A RpcClient is created with a handler for the call return by RpcServers:
.. doxygentypedef:: ib_Rpc_ResultHandler_t

RpcServers
~~~~~~~~~~
.. doxygenfunction:: ib_Rpc_Server_Create
.. doxygenfunction:: ib_Rpc_Server_SubmitResult

A RpcServers is created with a handler to process incoming calls by RpcClients:
.. doxygentypedef:: ib_Rpc_CallHandler_t

RpcServer Discovery
~~~~~~~~~~~~~~~~~~~

A participant can poll for already known RpcServers:

.. doxygenfunction:: ib_Rpc_DiscoverServers

The method takes a handler with the discovery results:

.. doxygentypedef:: ib_Rpc_DiscoveryResultHandler_t

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Rpc_ExchangeFormat
   :members:
.. doxygenstruct:: ib_Rpc_DiscoveryResultList
   :members:
.. doxygentypedef:: ib_Rpc_CallHandle
.. doxygentypedef:: ib_Rpc_CallStatus

FlexRay API
-----------
The FlexRay API consists of the following parts:

* The FlexRay controller
* The FlexRay controller and TX buffer configuration
* A set of FlexRay specific messages, each with its own function to register callbacks: 
  Message, MessageAck, Wakeup, ControllerStatus, PocStatus, Symbol, SymbolAck, CycleStart
  
 
FlexRay Controller
~~~~~~~~~~~~~~~~~~
A FlexRay controller interacts with the configured FlexRay bus and sends FlexRay frames and other events on it's own behalf.
Note that it is not possible to explicitly send frames or other events, nor exist any API functions to construct these events.
This is because the FlexRay bus works completely time triggered, sending preconfigured frames.
To configure these frames, the API provides functions to manipulate TX buffers.

**The following functions can be used to create a FlexRay controller and manipulate it's configuration:**

.. doxygenfunction:: ib_FlexRay_Controller_Create
.. doxygenfunction:: ib_FlexRay_ControllerConfig_Create
.. doxygenfunction:: ib_FlexRay_Append_TxBufferConfig
.. doxygenfunction:: ib_FlexRay_Controller_Configure
.. doxygenfunction:: ib_FlexRay_Controller_ReconfigureTxBuffer
.. doxygenfunction:: ib_FlexRay_Controller_UpdateTxBuffer

**The following function can be used to manipulate the controller's state by triggering chi commands:**

.. doxygenfunction:: ib_FlexRay_Controller_ExecuteCmd

**The following set of functions can be used to register event handlers to the controller:**

.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterWakeupHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterPocStatusHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterCycleStartHandler

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_FlexRay_Message
   :members:
.. doxygenstruct:: ib_FlexRay_MessageAck
   :members:
.. doxygenstruct:: ib_FlexRay_Symbol
   :members:
.. doxygenstruct:: ib_FlexRay_CycleStart
   :members:
.. doxygenstruct:: ib_FlexRay_ControllerStatus
   :members:
.. doxygenstruct:: ib_FlexRay_PocStatus
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_FlexRay_MacroTick
.. doxygentypedef:: ib_FlexRay_MicroTick
.. doxygentypedef:: ib_FlexRay_ClockPeriod
.. doxygentypedef:: ib_FlexRay_Channel
.. doxygentypedef:: ib_FlexRay_SymbolPattern
.. doxygentypedef:: ib_FlexRay_ChiCommand
.. doxygentypedef:: ib_FlexRay_TransmissionMode
.. doxygentypedef:: ib_FlexRay_PocState
.. doxygentypedef:: ib_FlexRay_SlotModeType
.. doxygentypedef:: ib_FlexRay_ErrorModeType
.. doxygentypedef:: ib_FlexRay_StartupStateType
.. doxygentypedef:: ib_FlexRay_WakeupStatusType


Lin API
-------
The Lin API for the C language provides communication in a Lin-Bus master/slave-architecture. 
The functionality is analogous to the C++ API described in :ref:`sec:lin`.
  
Lin Controller
~~~~~~~~~~~~~~

**A Lin controller is created with the following function:**

.. doxygenfunction:: ib_Lin_Controller_Create

**It's configuration is handed over to the function:**

.. doxygenfunction:: ib_Lin_Controller_Init

**The following set of functions can be used to register event handlers on the controller:**

.. doxygenfunction:: ib_Lin_Controller_RegisterFrameStatusHandler
.. doxygenfunction:: ib_Lin_Controller_RegisterGoToSleepHandler
.. doxygenfunction:: ib_Lin_Controller_RegisterWakeupHandler

**The following functions operate on a configured controller:**

.. doxygenfunction:: ib_Lin_Controller_Status
.. doxygenfunction:: ib_Lin_Controller_SendFrame
.. doxygenfunction:: ib_Lin_Controller_SendFrameWithTimestamp
.. doxygenfunction:: ib_Lin_Controller_SendFrameHeader
.. doxygenfunction:: ib_Lin_Controller_SendFrameHeaderWithTimestamp
.. doxygenfunction:: ib_Lin_Controller_SetFrameResponse
.. doxygenfunction:: ib_Lin_Controller_SetFrameResponses
.. doxygenfunction:: ib_Lin_Controller_GoToSleep
.. doxygenfunction:: ib_Lin_Controller_GoToSleepInternal
.. doxygenfunction:: ib_Lin_Controller_Wakeup
.. doxygenfunction:: ib_Lin_Controller_WakeupInternal

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib_Lin_ControllerConfig
   :members:
.. doxygenstruct:: ib_Lin_Frame
   :members:
.. doxygenstruct:: ib_Lin_FrameResponse
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_Lin_Controller
.. doxygentypedef:: ib_Lin_ControllerStatus
.. doxygentypedef:: ib_Lin_ControllerMode
.. doxygentypedef:: ib_Lin_BaudRate
.. doxygentypedef:: ib_Lin_FrameResponseMode
.. doxygentypedef:: ib_Lin_Id
.. doxygentypedef:: ib_Lin_ChecksumModel
.. doxygentypedef:: ib_Lin_FrameResponseType
.. doxygentypedef:: ib_Lin_FrameStatus
.. doxygentypedef:: ib_Lin_DataLength

The Logger API can be used to write log messages.

.. doxygenfunction:: ib_SimulationParticipant_GetLogger
.. doxygenfunction:: ib_Logger_Log

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib_LoggingLevel
