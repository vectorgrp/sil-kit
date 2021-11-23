.. _sec:capi:

===================
Experimental C API
===================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp


Using the Experimental C API
-------------------------

VIB Entry Point and API Organization
~~~~~~~~~~~~~~~~~~~~

The main entry point of the C API is the function to obtain a ib_SimulationParticipant*::

    ib_SimulationParticipant* participant = NULL;
    const char* jsonString = "{ ... }";
    const char* participantName = "CanWriter";
    const char* domainId = "1";
    ib_ReturnCode result = ib_SimulationParticipant_create(&participant, jsonString,
                                                            participantName, domainId);

All further services of the C API of the IntegrationBus are requested through this SimulationParticipant.

Entities obtained through the SimulationParticipant must not be destroyed/deleted by the user of the API.
All entities, that are provided through the API expect for the ib_SimulationParticipant are destroyed through
the internals of the Integration Bus implementation.

After creation of a SimulationParticipant it must be ensured that eventually ib_Participant_destroy is called
with the corresponding pointer to the ib_SimulationParticipant entity.


API and Data Type Reference
--------------------------------------------------
General API
~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_SimulationParticipant_create
.. doxygenfunction:: ib_SimulationParticipant_destroy
.. doxygenfunction:: ib_ReturnCodeToString
.. doxygenfunction:: ib_GetLastErrorString

Can API
~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: ib_CanController_create
.. doxygenfunction:: ib_CanController_Start
.. doxygenfunction:: ib_CanController_Stop
.. doxygenfunction:: ib_CanController_Reset
.. doxygenfunction:: ib_CanController_Sleep
.. doxygenfunction:: ib_CanController_SendFrame
.. doxygenfunction:: ib_CanController_SetBaudRate
.. doxygenfunction:: ib_CanController_RegisterTransmitStatusHandler
.. doxygenfunction:: ib_CanController_RegisterReceiveMessageHandler
.. doxygenfunction:: ib_CanController_RegisterStateChangedHandler
.. doxygenfunction:: ib_CanController_RegisterErrorStateChangedHandler

Ethernet API
~~~~~~~~~~~~~~~~~~~~
The Ethernet API consists out of two main parts:

# The Ethernet controller
# The Ethernet frame

Ethernet Controller
++++++++++++++++++++

The Ethernet controller interacts with the corresponding Ethernet bus and send Ethernet frames.
The Ethernet frames are the single messages/frames, that are transmitted over the Ethernet bus.

.. doxygenfunction:: ib_EthernetController_create
.. doxygenfunction:: ib_EthernetController_Activate
.. doxygenfunction:: ib_EthernetController_Deactivate
.. doxygenfunction:: ib_EthernetController_RegisterReceiveMessageHandler
.. doxygenfunction:: ib_EthernetController_RegisterFrameAckHandler
.. doxygenfunction:: ib_EthernetController_RegisterStateChangedHandler
.. doxygenfunction:: ib_EthernetController_RegisterBitRateChangedHandler
.. doxygenfunction:: ib_EthernetController_SendFrame


Ethernet Frame
++++++++++++++++++++

The ib_EthernetFrame corresponds to an ethernet raw frame.
A basic frame consists out of the destination mac, the source mac, the ethertype and a payload.
The union type within the ib_EthernetFrame helps when manual construction of a frame is necessary (frameHeader, frameHeaderVlanTagged).

.. note:: For an example of manual frame construction one can refer to the C Ethernet demo.

Data API
~~~~~~~~~~~~~~~~~~~~
The Data API provides data publish and subscribe functionalities to the Integration Bus. 
It consists out of DataPublishers and DataSubscribers.

.. doxygenfunction:: ib_DataPublisher_create
.. doxygenfunction:: ib_DataSubscriber_create
.. doxygenfunction:: ib_DataPublisher_Publish
.. doxygenfunction:: ib_DataSubscriber_SetReceiveDataHandler
.. doxygenfunction:: ib_SimulationParticipant_GetAllPublishersOfSimulation
.. doxygenfunction:: ib_SimulationParticipant_GetAllSubscribersOfSimulation

FlexRay API
~~~~~~~~~~~~~~~~~~~~
The FlexRay API consists of the following parts:

# The FlexRay controller
# The FlexRay controller and TX buffer configuration
# A set of FlexRay specific messages, each with its own function to register callbacks:
   Message, MessageAck, Wakeup, ControllerStatus, PocStatus, Symbol, SymbolAck, CycleStart
  
 
FlexRay Controller
++++++++++++++++++++
A FlexRay controller interacts with the configured FlexRay bus and sends FlexRay frames and other events on it's own behalf.
Note that it is not possible to explicitly send frames or other events, nor exist any API functions to construct these events.
This is because the FlexRay bus works completely time triggered, sending preconfigured frames.
To configure these frames, the API provides functions to manipulate TX buffers.

The following functions can be used to create a FlexRay controller and manipulate it's configuration:

.. doxygenfunction:: ib_FlexRay_Controller_Create
.. doxygenfunction:: ib_FlexRay_ControllerConfig_Create
.. doxygenfunction:: ib_FlexRay_Append_TxBufferConfig
.. doxygenfunction:: ib_FlexRay_Controller_Configure
.. doxygenfunction:: ib_FlexRay_Controller_ReconfigureTxBuffer
.. doxygenfunction:: ib_FlexRay_Controller_UpdateTxBuffer

The following function can be used to manipulate the controller's state by triggering chi commands:

.. doxygenfunction:: ib_FlexRay_Controller_ExecuteCmd

The following set of functions can be used to register event handlers to the controller:

.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterMessageAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterWakeupHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterPocStatusHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterSymbolAckHandler
.. doxygenfunction:: ib_FlexRay_Controller_RegisterCycleStartHandler

Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
