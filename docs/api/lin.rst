.. _sec:lin:

===================
LIN Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>`
.. |CreateLinController| replace:: :cpp:func:`CreateLinController<ib::mw::IParticipant::CreateLinController()>`
.. |ILinController| replace:: :cpp:class:`ILinController<ib::sim::eth::ILinController>`

.. |Init| replace:: :cpp:func:`Init()<ib::sim::lin::ILinController::Init>`
.. |SendFrame| replace:: :cpp:func:`SendFrame()<ib::sim::lin::ILinController::SendFrame>`
.. |SendFrameHeader| replace:: :cpp:func:`SendFrameHeader()<ib::sim::lin::ILinController::SendFrameHeader>`
.. |SetFrameResponse| replace:: :cpp:func:`SetFrameResponse()<ib::sim::lin::ILinController::SetFrameResponse>`

.. |Wakeup| replace:: :cpp:func:`Wakeup()<ib::sim::lin::ILinController::Wakeup>`
.. |GoToSleep| replace:: :cpp:func:`GoToSleep()<ib::sim::lin::ILinController::GoToSleep>`

.. |AddFrameStatusHandler| replace:: :cpp:type:`AddFrameStatusHandler()<ib::sim::lin::ILinController::AddFrameStatusHandler>`
.. |AddGoToSleepHandler| replace:: :cpp:type:`AddGoToSleepHandler()<ib::sim::lin::ILinController::AddGoToSleepHandler>`
.. |AddWakeupHandler| replace:: :cpp:type:`AddWakeupHandler()<ib::sim::lin::ILinController::AddWakeupHandler>`
.. |AddFrameResponseUpdateHandler| replace:: :cpp:type:`AddFrameResponseUpdateHandler()<ib::sim::lin::ILinController::AddFrameResponseUpdateHandler>`

.. |FrameStatusHandler| replace:: :cpp:type:`FrameStatusHandler<ib::sim::lin::ILinController::FrameStatusHandler>`
.. |GoToSleepHandler| replace:: :cpp:type:`GoToSleepHandler<ib::sim::lin::ILinController::GoToSleepHandler>`
.. |WakeupHandler| replace:: :cpp:type:`WakeupHandler<ib::sim::lin::ILinController::WakeupHandler>`
.. |FrameResponseUpdateHandler| replace:: :cpp:type:`FrameResponseUpdateHandler<ib::sim::lin::ILinController::FrameResponseUpdateHandler>`

.. |LinFrameStatusEvent| replace:: :cpp:class:`LinFrameStatusEvent<ib::sim::lin::LinFrameStatusEvent>`
.. |LinGoToSleepEvent| replace:: :cpp:class:`LinGoToSleepEvent<ib::sim::lin::LinGoToSleepEvent>`
.. |LinWakeupEvent| replace:: :cpp:class:`LinWakeupEvent<ib::sim::lin::LinWakeupEvent>`
.. |LinFrameResponseUpdateEvent| replace:: :cpp:class:`LinFrameResponseUpdateEvent<ib::sim::lin::LinFrameResponseUpdateEvent>`

.. |ControllerConfig| replace:: :cpp:class:`ControllerConfig<ib::sim::lin::ControllerConfig>`
.. |FrameResponse| replace:: :cpp:class:`FrameResponse<ib::sim::lin::FrameResponse>`
.. |LinFrame| replace:: :cpp:class:`LinFrame<ib::sim::lin::LinFrame>`

.. |ControllerMode| replace:: :cpp:class:`ControllerMode<ib::sim::lin::ControllerMode>`
.. |ControllerMode_Master| replace:: :cpp:enumerator:`ControllerMode::Master<ib::sim::lin::Master>`
.. |ControllerMode_Slave| replace:: :cpp:enumerator:`ControllerMode::Slave<ib::sim::lin::Slave>`

.. |FrameResponseType_MasterResponse| replace:: :cpp:enumerator:`FrameResponseType::MasterResponse<ib::sim::lin::MasterResponse>`
.. |FrameResponseType_SlaveResponse| replace:: :cpp:enumerator:`FrameResponseType::SlaveResponse<ib::sim::lin::SlaveResponse>`

.. |FrameResponseMode_Rx| replace:: :cpp:enumerator:`FrameResponseMode::Rx<ib::sim::lin::Rx>`

.. |FrameStatus| replace:: :cpp:class:`FrameStatus<ib::sim::lin::FrameStatus>`
.. |FrameStatus_LIN_TX_ERROR| replace:: :cpp:enumerator:`FrameStatus::LIN_TX_ERROR<ib::sim::lin::LIN_TX_ERROR>`
.. |FrameStatus_LIN_RX_ERROR| replace:: :cpp:enumerator:`FrameStatus::LIN_RX_ERROR<ib::sim::lin::LIN_RX_ERROR>`
.. |FrameStatus_LIN_RX_NO_RESPONSE| replace:: :cpp:enumerator:`FrameStatus::LIN_RX_NO_RESPONSE<ib::sim::lin::LIN_RX_NO_RESPONSE>`

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
|ControllerConfig| and passing it to |Init|.

At a minimum, the |ControllerMode| must be set to either |ControllerMode_Master| or |ControllerMode_Slave|. If the VIBE
NetworkSimulator is used, also a baud rate must be specified. In addition, the |ControllerConfig| allows providing
an initial set of |FrameResponse|, which is particularly useful for LIN slaves.

The following example configures a LIN controller as a LIN slave with a baud rate of 20'000 baud. Furthermore, LIN ID 
0x11 is configured for transmission::

    FrameResponse response;
    response.frame.id = 0x11;
    response.frame.checksumModel = ChecksumModel::Enhanced;
    response.frame.dataLength = 8;
    response.frame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    response.responseMode = FrameResponseMode::Rx;

    ControllerConfig slaveConfig;
    slaveConfig.controllerMode = ControllerMode::Slave;
    slaveConfig.baudRate = 20000;
    slaveConfig.frameResponses.push_back(response);

    linController->Init(slaveConfig);

Note that |Init| should be called in the InitHandler of a ParticipantController.

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
  masterFrame.checksumModel = ChecksumModel::Enhanced;

To be notified for the success or failure of the transmission, a |FrameStatusHandler| should be registered using 
|AddFrameStatusHandler|::
  
  // Register FrameStatusHandler to receive data from the LIN slave
  auto frameStatusHandler =
      [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
  master->AddFrameStatusHandler(frameStatusHandler);

The transmission of the frame can be initiated using either the AUTOSAR interface or the low-level, non-AUTOSAR 
interface:

    1. Using the AUTOSAR interface, the frame can be sent in one call with |FrameResponseType_MasterResponse| indicating 
       that the master provides both header and response::

         // send the frame using the AUTOSAR interface
         master->SendFrame(masterFrame, FrameResponseType::MasterResponse);

    2. Using the low-level interface, the frame is sent in two steps. First a FrameResponse is configured with 
       TxUnconditional, and then a LIN header with the corresponding id is sent::

         // Setup the master response
         master->SetFrameResponse(masterFrame, FrameResponseMode::TxUnconditional);

         // Transmit the frame header, the response will be transmitted automatically.
         master->SendFrameHeader(0x10);


A successful transmission is confirmed via the registered callback, e.g.::

  frameStatusHandler(master, frameStatusEvent);
  // With:
  // frameStatusEvent.timestamp: timeEndOfFrame;
  // frameStatusEvent.frame: masterFrame;
  // frameStatusEvent.status: FrameStatus::LIN_TX_OK;

.. admonition:: Note

    If another controller also has ID 0x10 configured for transmission, a collision occurs on the bus, which is 
    indicated by |FrameStatus_LIN_TX_ERROR|.

.. admonition:: Note

    The end of frame timestamp is only valid when using the VIBE NetworkSimulator.

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
    frameRequest.checksumModel = ChecksumModel::Enhanced;
    frameRequest.dataLength = 8;

Again, the transmission can be initiated using either the AUTOSAR interface or the low-level, non-AUTOSAR interface.

    1. Using the AUTOSAR interface, the frame can be sent in one call. The |FrameResponseType_SlaveResponse| indicates 
       that the master only provides the frame header and expects a slave to provide the response::
    
        // Initiate the transmission
        master->SendFrame(frameRequest, FrameResponseType::SlaveResponse);

    2. Alternatively, the transmission can be initiated using the low-level interface. This requires first to configure 
       a FrameResponse with |FrameResponseMode_Rx|, and then sending a LIN header with the corresponding id::

        // Configure LIN ID 0x11 for reception
        master->SetFrameResponse(frameRequest, FrameResponseMode::Rx);
    
        // Initiate the transmission, a FrameResponse must be setup by a LIN slave
        master->SendFrameHeader(0x11);

The end of the transmission will be confirmed by a call to the registered |FrameStatusHandler|. The incoming 
|LinFrameStatusEvent| holds a |FrameStatus| indicating the success or failure of the transmission.

If a LIN slave has previously setup a matching |FrameResponse| for transmission, the registered |FrameStatusHandler| 
will deliver a |LinFrameStatusEvent| as follows::

    frameStatusHandler(master, frameStatusEvent);
    // With:
    // frameStatusEvent.timestamp: timeEndOfFrame;
    // frameStatusEvent.frame: slaveFrame;
    // frameStatusEvent.status: FrameStatus::LIN_RX_OK;

.. admonition:: Note

    If the frame response provided by the |LinFrame| does not match both expected dataLength and checksumModel, or if 
    more than one slave provided a response, the |FrameStatus_LIN_RX_ERROR| will be used. If no LIN slave provides a 
    frame response, the |FrameStatus_LIN_RX_NO_RESPONSE| will be used.

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
.. doxygenclass:: ib::sim::lin::ILinController
   :members:

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::lin::LinFrame
   :members:
.. doxygenstruct:: ib::sim::lin::FrameResponse
   :members:
.. doxygenstruct:: ib::sim::lin::ControllerConfig
   :members:
.. doxygenstruct:: ib::sim::lin::LinFrameStatusEvent
   :members:
.. doxygenstruct:: ib::sim::lin::LinWakeupEvent
   :members:
.. doxygenstruct:: ib::sim::lin::LinGoToSleepEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib::sim::lin::LinIdT
.. doxygenenum:: ib::sim::lin::ChecksumModel
.. doxygentypedef:: ib::sim::lin::DataLengthT
.. doxygenenum:: ib::sim::lin::FrameResponseType
.. doxygenenum:: ib::sim::lin::FrameResponseMode
.. doxygenenum:: ib::sim::lin::FrameStatus
.. doxygenenum:: ib::sim::lin::ControllerMode
.. doxygentypedef:: ib::sim::lin::BaudRateT
.. doxygenenum:: ib::sim::lin::ControllerStatus
     

Usage Examples
--------------

This section contains more complex examples that show the interaction of two or more LIN controllers. Although the LIN 
controllers would typically belong to different participants and reside in different processes, their interaction is
shown here sequentially to demonstrate cause and effect.

Assumptions:

- *master*, *slave*, *slave1*, and *slave2* are of type |ILinController|.
- *timeEndOfFrame* indicates the end of frame time stamp when using the VIBE NetworkSimulator. Otherwise the value of 
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

This example shows how the |FrameResponseUpdateHandler| provides direct access to the |FrameResponse| configuration of 
all slaves. It is primarily intended for diagnostic purposes and not required for regular operation of a LIN controller.

.. literalinclude::
   examples/lin/FrameResponseUpdateHandler.cpp
   :language: cpp
