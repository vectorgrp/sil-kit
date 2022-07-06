.. _sec:lin:

===================
LIN Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateLinController| replace:: :cpp:func:`CreateLinController<SilKit::IParticipant::CreateLinController()>`
.. |ILinController| replace:: :cpp:class:`ILinController<SilKit::Services::Ethernet::ILinController>`

.. |Init| replace:: :cpp:func:`Init()<SilKit::Services::Lin::ILinController::Init>`
.. |SendFrame| replace:: :cpp:func:`SendFrame()<SilKit::Services::Lin::ILinController::SendFrame>`
.. |SendFrameHeader| replace:: :cpp:func:`SendFrameHeader()<SilKit::Services::Lin::ILinController::SendFrameHeader>`
.. |SetFrameResponse| replace:: :cpp:func:`SetFrameResponse()<SilKit::Services::Lin::ILinController::SetFrameResponse>`

.. |Wakeup| replace:: :cpp:func:`Wakeup()<SilKit::Services::Lin::ILinController::Wakeup>`
.. |GoToSleep| replace:: :cpp:func:`GoToSleep()<SilKit::Services::Lin::ILinController::GoToSleep>`

.. |AddFrameStatusHandler| replace:: :cpp:type:`AddFrameStatusHandler()<SilKit::Services::Lin::ILinController::AddFrameStatusHandler>`
.. |AddGoToSleepHandler| replace:: :cpp:type:`AddGoToSleepHandler()<SilKit::Services::Lin::ILinController::AddGoToSleepHandler>`
.. |AddWakeupHandler| replace:: :cpp:type:`AddWakeupHandler()<SilKit::Services::Lin::ILinController::AddWakeupHandler>`
.. |AddFrameResponseUpdateHandler| replace:: :cpp:type:`AddFrameResponseUpdateHandler()<SilKit::Services::Lin::ILinController::AddFrameResponseUpdateHandler>`

.. |RemoveFrameStatusHandler| replace:: :cpp:type:`RemoveFrameStatusHandler()<SilKit::Services::Lin::ILinController::RemoveFrameStatusHandler>`
.. |RemoveGoToSleepHandler| replace:: :cpp:type:`RemoveGoToSleepHandler()<SilKit::Services::Lin::ILinController::RemoveGoToSleepHandler>`
.. |RemoveWakeupHandler| replace:: :cpp:type:`RemoveWakeupHandler()<SilKit::Services::Lin::ILinController::RemoveWakeupHandler>`
.. |RemoveFrameResponseUpdateHandler| replace:: :cpp:type:`RemoveFrameResponseUpdateHandler()<SilKit::Services::Lin::ILinController::RemoveFrameResponseUpdateHandler>`

.. |FrameStatusHandler| replace:: :cpp:type:`FrameStatusHandler<SilKit::Services::Lin::ILinController::FrameStatusHandler>`
.. |GoToSleepHandler| replace:: :cpp:type:`GoToSleepHandler<SilKit::Services::Lin::ILinController::GoToSleepHandler>`
.. |WakeupHandler| replace:: :cpp:type:`WakeupHandler<SilKit::Services::Lin::ILinController::WakeupHandler>`
.. |FrameResponseUpdateHandler| replace:: :cpp:type:`FrameResponseUpdateHandler<SilKit::Services::Lin::ILinController::FrameResponseUpdateHandler>`

.. |LinFrameStatusEvent| replace:: :cpp:class:`LinFrameStatusEvent<SilKit::Services::Lin::LinFrameStatusEvent>`
.. |LinGoToSleepEvent| replace:: :cpp:class:`LinGoToSleepEvent<SilKit::Services::Lin::LinGoToSleepEvent>`
.. |LinWakeupEvent| replace:: :cpp:class:`LinWakeupEvent<SilKit::Services::Lin::LinWakeupEvent>`
.. |LinFrameResponseUpdateEvent| replace:: :cpp:class:`LinFrameResponseUpdateEvent<SilKit::Services::Lin::LinFrameResponseUpdateEvent>`

.. |LinControllerConfig| replace:: :cpp:class:`LinControllerConfig<SilKit::Services::Lin::LinControllerConfig>`
.. |LinFrameResponse| replace:: :cpp:class:`LinFrameResponse<SilKit::Services::Lin::LinFrameResponse>`
.. |LinFrame| replace:: :cpp:class:`LinFrame<SilKit::Services::Lin::LinFrame>`

.. |LinControllerMode| replace:: :cpp:enum:`LinControllerMode<SilKit::Services::Lin::LinControllerMode>`
.. |LinControllerMode_Master| replace:: :cpp:enumerator:`LinControllerMode::Master<SilKit::Services::Lin::Master>`
.. |LinControllerMode_Slave| replace:: :cpp:enumerator:`LinControllerMode::Slave<SilKit::Services::Lin::Slave>`

.. |LinFrameResponseType_MasterResponse| replace:: :cpp:enumerator:`LinFrameResponseType::MasterResponse<SilKit::Services::Lin::MasterResponse>`
.. |LinFrameResponseType_SlaveResponse| replace:: :cpp:enumerator:`LinFrameResponseType::SlaveResponse<SilKit::Services::Lin::SlaveResponse>`

.. |LinFrameResponseMode_Rx| replace:: :cpp:enumerator:`LinFrameResponseMode::Rx<SilKit::Services::Lin::Rx>`

.. |LinFrameStatus| replace:: :cpp:enum:`LinFrameStatus<SilKit::Services::Lin::LinFrameStatus>`
.. |LinFrameStatus_LIN_TX_ERROR| replace:: :cpp:enumerator:`LinFrameStatus::LIN_TX_ERROR<SilKit::Services::Lin::LIN_TX_ERROR>`
.. |LinFrameStatus_LIN_RX_ERROR| replace:: :cpp:enumerator:`LinFrameStatus::LIN_RX_ERROR<SilKit::Services::Lin::LIN_RX_ERROR>`
.. |LinFrameStatus_LIN_RX_NO_RESPONSE| replace:: :cpp:enumerator:`LinFrameStatus::LIN_RX_NO_RESPONSE<SilKit::Services::Lin::LIN_RX_NO_RESPONSE>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<SilKit::Services::HandlerId>`

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp
   
Using the LIN Controller
-------------------------

The LIN Service API provides a LIN bus abstraction through the |ILinController| interface.
A LIN controller is created by calling |CreateLinController| given a controller name and (optional) network 
name::

  auto* linMasterController = participant->CreateLinController("LinMaster", "LIN1");

LIN controllers will only communicate within the same network. If no network name is provided, the controller name
will be used as the network name.

Initialization
~~~~~~~~~~~~~~

Before the LIN Controller can be used, it must be initialized. The initialization is performed by setting up a
|LinControllerConfig| and passing it to |Init|.

At a minimum, the |LinControllerMode| must be set to either |LinControllerMode_Master| or |LinControllerMode_Slave| 
and the baud rate must be specified. In addition, the |LinControllerConfig| allows providing
an initial set of |LinFrameResponse|, which is particularly useful for LIN slaves.

The following example configures a LIN controller as a LIN slave with a baud rate of 20'000 baud. Furthermore, LIN ID 
0x11 is configured for transmission::

    LinFrameResponse response;
    response.frame.id = 0x11;
    response.frame.checksumModel = LinChecksumModel::Enhanced;
    response.frame.dataLength = 8;
    response.frame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    response.responseMode = LinFrameResponseMode::Rx;

    LinControllerConfig slaveConfig;
    slaveConfig.controllerMode = LinControllerMode::Slave;
    slaveConfig.baudRate = 20000;
    slaveConfig.frameResponses.push_back(response);

    linController->Init(slaveConfig);

Note that |Init| should be called in the CommunicationReadyHandler of the LifecycleService.

Initiating LIN Transmissions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Data is transfered in the form of a |LinFrame|. A LIN master can initiate the transmission of a frame using the AUTOSAR 
API |SendFrame| or using the low-level, non-AUTOSAR API |SendFrameHeader| which requires that a corresponding frame
response was configured before at any LIN controller using |SetFrameResponse|.

Sending Data to Slaves
______________________

To send data, a |LinFrame| must be setup with the LIN ID, the data to be transmitted, and the desired checksum model::

  // Prepare a frame with id 0x10 for transmission
  LinFrame masterFrame;
  masterFrame.id = 0x10;
  masterFrame.dataLength = 8;
  masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
  masterFrame.checksumModel = LinChecksumModel::Enhanced;

To be notified for the success or failure of the transmission, a |FrameStatusHandler| should be registered using 
|AddFrameStatusHandler|::
  
  // Register FrameStatusHandler to receive data from the LIN slave
  auto frameStatusHandler =
      [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
  master->AddFrameStatusHandler(frameStatusHandler);

The transmission of the frame can be initiated using either the AUTOSAR interface or the low-level, non-AUTOSAR 
interface:

    1. Using the AUTOSAR interface, the frame can be sent in one call with |LinFrameResponseType_MasterResponse| indicating 
       that the master provides both header and response::

         // send the frame using the AUTOSAR interface
         master->SendFrame(masterFrame, LinFrameResponseType::MasterResponse);

    2. Using the low-level interface, the frame is sent in two steps. First a |LinFrameResponse| is configured with 
       TxUnconditional, and then a LIN header with the corresponding id is sent::

         // Setup the master response
         master->SetFrameResponse(masterFrame, LinFrameResponseMode::TxUnconditional);

         // Transmit the frame header, the response will be transmitted automatically.
         master->SendFrameHeader(0x10);


A successful transmission is confirmed via the registered callback, e.g.::

  frameStatusHandler(master, frameStatusEvent);
  // With:
  // frameStatusEvent.timestamp: timeEndOfFrame;
  // frameStatusEvent.frame: masterFrame;
  // frameStatusEvent.status: LinFrameStatus::LIN_TX_OK;

.. admonition:: Note

    If another controller also has ID 0x10 configured for transmission, a collision occurs on the bus, which is 
    indicated by |LinFrameStatus_LIN_TX_ERROR|.

.. admonition:: Note

    The end of frame timestamp is only valid when using the detailed simulation.

Receiving Data from a Slave
___________________________

To receive data from a LIN slave, a |FrameStatusHandler| must be registered using |AddFrameStatusHandler|, which is 
called by the LIN controller whenever a frame is received::

    // Register FrameStatusHandler to receive data from the LIN slave
    auto frameStatusHandler =
        [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
    master->AddFrameStatusHandler(frameStatusHandler);

To initiate the data transfer from slave to master, the master must provide the LIN ID as well as the expected data 
length and checksum model::

    // Prepare to receive a frame with id 0x11
    LinFrame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = LinChecksumModel::Enhanced;
    frameRequest.dataLength = 8;

Again, the transmission can be initiated using either the AUTOSAR interface or the low-level, non-AUTOSAR interface.

    1. Using the AUTOSAR interface, the frame can be sent in one call. The |LinFrameResponseType_SlaveResponse| indicates 
       that the master only provides the frame header and expects a slave to provide the response::
    
        // Initiate the transmission
        master->SendFrame(frameRequest, LinFrameResponseType::SlaveResponse);

    2. Alternatively, the transmission can be initiated using the low-level interface. This requires first to configure 
       a |LinFrameResponse| with |LinFrameResponseMode_Rx|, and then sending a LIN header with the corresponding id::

        // Configure LIN ID 0x11 for reception
        master->SetFrameResponse(frameRequest, LinFrameResponseMode::Rx);
    
        // Initiate the transmission, a LinFrameResponse must be setup by a LIN slave
        master->SendFrameHeader(0x11);

The end of the transmission will be confirmed by a call to the registered |FrameStatusHandler|. The incoming 
|LinFrameStatusEvent| holds a |LinFrameStatus| indicating the success or failure of the transmission.

If a LIN slave has previously setup a matching |LinFrameResponse| for transmission, the registered |FrameStatusHandler| 
will deliver a |LinFrameStatusEvent| as follows::

    frameStatusHandler(master, frameStatusEvent);
    // With:
    // frameStatusEvent.timestamp: timeEndOfFrame;
    // frameStatusEvent.frame: slaveFrame;
    // frameStatusEvent.status: LinFrameStatus::LIN_RX_OK;

.. admonition:: Note

    If the frame response provided by the |LinFrame| does not match both expected dataLength and checksumModel, or if 
    more than one slave provided a response, the |LinFrameStatus_LIN_RX_ERROR| will be used. If no LIN slave provides a 
    frame response, the |LinFrameStatus_LIN_RX_NO_RESPONSE| will be used.

Managing the event handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameStatusHandler|
- |RemoveGoToSleepHandler|
- |RemoveWakeupHandler|
- |RemoveFrameResponseUpdateHandler|

Message Tracing
~~~~~~~~~~~~~~~

.. admonition:: Note

  Currently the Message Tracing functionality is not available, but it will be reintegrated in the future.

The LinController supports message tracing in MDF4 format. This is provided by the :ref:`VIBE MDF4Tracing<mdf4tracing>`
extension. Refer to the :ref:`sec:cfg-participant-tracing` configuration section for usage instructions.
     
API and Data Type Reference
---------------------------
LIN Controller API
~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Services::Lin::ILinController
   :members:

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit::Services::Lin::LinFrame
   :members:
.. doxygenstruct:: SilKit::Services::Lin::LinFrameResponse
   :members:
.. doxygenstruct:: SilKit::Services::Lin::LinControllerConfig
   :members:
.. doxygenstruct:: SilKit::Services::Lin::LinFrameStatusEvent
   :members:
.. doxygenstruct:: SilKit::Services::Lin::LinWakeupEvent
   :members:
.. doxygenstruct:: SilKit::Services::Lin::LinGoToSleepEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: SilKit::Services::Lin::LinIdT
.. doxygenenum:: SilKit::Services::Lin::LinChecksumModel
.. doxygentypedef:: SilKit::Services::Lin::LinDataLengthT
.. doxygenenum:: SilKit::Services::Lin::LinFrameResponseType
.. doxygenenum:: SilKit::Services::Lin::LinFrameResponseMode
.. doxygenenum:: SilKit::Services::Lin::LinFrameStatus
.. doxygenenum:: SilKit::Services::Lin::LinControllerMode
.. doxygentypedef:: SilKit::Services::Lin::LinBaudRateT
.. doxygenenum:: SilKit::Services::Lin::LinControllerStatus

Usage Examples
--------------

This section contains more complex examples that show the interaction of two or more LIN controllers. Although the LIN 
controllers would typically belong to different participants and reside in different processes, their interaction is
shown here sequentially to demonstrate cause and effect.

Assumptions:

- *master*, *slave*, *slave1*, and *slave2* are of type |ILinController|.
- *timeEndOfFrame* indicates the end of frame time stamp when using the detiled simulation. Otherwise the value of 
  *timeEndofFrame* is undefined.
- *UseAutosarInterface* is a boolean variable that indicates whether to use the AUTOSAR API or the non-AUTOSAR API. 
  It will most likely not be used in practice and it merely intended to show the different usages of the API.
  
Successful Transmission from Master to Slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN master to a LIN slave. The transmission must be initiated by 
the master.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_OK.cpp
   :language: cpp
   
Successful Transmission from Slave to Master
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN Slave to a LIN master. The transmission must be initiated by 
the master.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_OK.cpp
   :language: cpp

Successful Transmission from Slave to Slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how data is transferred from one LIN slave to another one. The data transfer must be initiated by a 
LIN master.

.. literalinclude::
   examples/lin/Slave_to_Slave_LIN_TX_OK.cpp
   :language: cpp
              
Erroneous Transmission from Master to Slave - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master atempts to send a Frame while there is slave that has configured a TX 
response for the same LIN ID.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_ERROR.cpp
   :language: cpp
   
Erroneous Transmission from Slave to Master - No Response
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission and no slave has configured a TX response for 
this LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_NO_RESPONSE.cpp
   :language: cpp

.. admonition:: Note

   For simplicity, this example only uses the AUTOSAR interface, the behavior and error code would be the same for the 
   non-AUTOSAR interface.

Erroneous Transmission from Slave to Master - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission where multiple slaves have configured TX 
responses for the same LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_ERROR.cpp
   :language: cpp

Go To Sleep
~~~~~~~~~~~

This example shows how to initiate a |GoToSleep| and when the controller switch from operational to sleep mode.

.. literalinclude::
   examples/lin/Go_To_Sleep.cpp
   :language: cpp

.. admonition:: Note
    
    The |GoToSleepHandler| is triggered even without configuring ID 0x3C for reception. However, the 
    |FrameStatusHandler| is only called if ID 0x3C is configured for reception.

Wake Up
~~~~~~~

This example shows how to |Wakeup| a LIN bus. The example assumes that both master and slave are currently in sleep mode.
I.e., the situation corresponds to the end of the previous example.

.. literalinclude::
   examples/lin/Wake_Up.cpp
   :language: cpp

FrameResponseUpdateHandler
~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how the |FrameResponseUpdateHandler| provides direct access to the |LinFrameResponse| configuration of 
all slaves. It is primarily intended for diagnostic purposes and not required for regular operation of a LIN controller.

.. literalinclude::
   examples/lin/FrameResponseUpdateHandler.cpp
   :language: cpp
