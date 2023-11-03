.. _chap:lin-service-api:

===================
LIN Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateLinController| replace:: :cpp:func:`CreateLinController<SilKit::IParticipant::CreateLinController()>`
.. |ILinController| replace:: :cpp:class:`ILinController<SilKit::Services::Lin::ILinController>`

.. |Init| replace:: :cpp:func:`Init()<SilKit::Services::Lin::ILinController::Init>`
.. |SendFrame| replace:: :cpp:func:`SendFrame()<SilKit::Services::Lin::ILinController::SendFrame>`
.. |SendFrameHeader| replace:: :cpp:func:`SendFrameHeader()<SilKit::Services::Lin::ILinController::SendFrameHeader>`
.. |UpdateTxBuffer| replace:: :cpp:func:`UpdateTxBuffer()<SilKit::Services::Lin::ILinController::UpdateTxBuffer>`
.. |SetFrameResponse| replace:: :cpp:func:`SetFrameResponse()<SilKit::Services::Lin::ILinController::SetFrameResponse>`

.. |Wakeup| replace:: :cpp:func:`Wakeup()<SilKit::Services::Lin::ILinController::Wakeup>`
.. |GoToSleep| replace:: :cpp:func:`GoToSleep()<SilKit::Services::Lin::ILinController::GoToSleep>`

.. |AddFrameStatusHandler| replace:: :cpp:func:`AddFrameStatusHandler()<SilKit::Services::Lin::ILinController::AddFrameStatusHandler>`
.. |AddGoToSleepHandler| replace:: :cpp:func:`AddGoToSleepHandler()<SilKit::Services::Lin::ILinController::AddGoToSleepHandler>`
.. |AddWakeupHandler| replace:: :cpp:func:`AddWakeupHandler()<SilKit::Services::Lin::ILinController::AddWakeupHandler>`

.. |RemoveFrameStatusHandler| replace:: :cpp:func:`RemoveFrameStatusHandler()<SilKit::Services::Lin::ILinController::RemoveFrameStatusHandler>`
.. |RemoveGoToSleepHandler| replace:: :cpp:func:`RemoveGoToSleepHandler()<SilKit::Services::Lin::ILinController::RemoveGoToSleepHandler>`
.. |RemoveWakeupHandler| replace:: :cpp:func:`RemoveWakeupHandler()<SilKit::Services::Lin::ILinController::RemoveWakeupHandler>`

.. |FrameStatusHandler| replace:: :cpp:type:`FrameStatusHandler<SilKit::Services::Lin::ILinController::FrameStatusHandler>`
.. |GoToSleepHandler| replace:: :cpp:type:`GoToSleepHandler<SilKit::Services::Lin::ILinController::GoToSleepHandler>`
.. |WakeupHandler| replace:: :cpp:type:`WakeupHandler<SilKit::Services::Lin::ILinController::WakeupHandler>`
.. |LinSlaveConfigurationHandler| replace:: :cpp:type:`LinSlaveConfigurationHandler<SilKit::Services::Lin::ILinController::LinSlaveConfigurationHandler>`

.. |LinFrameStatusEvent| replace:: :cpp:class:`LinFrameStatusEvent<SilKit::Services::Lin::LinFrameStatusEvent>`
.. |LinGoToSleepEvent| replace:: :cpp:class:`LinGoToSleepEvent<SilKit::Services::Lin::LinGoToSleepEvent>`
.. |LinWakeupEvent| replace:: :cpp:class:`LinWakeupEvent<SilKit::Services::Lin::LinWakeupEvent>`
.. |LinSlaveConfigurationEvent| replace:: :cpp:class:`LinSlaveConfigurationEvent<SilKit::Services::Lin::LinSlaveConfigurationEvent>`

.. |LinControllerConfig| replace:: :cpp:class:`LinControllerConfig<SilKit::Services::Lin::LinControllerConfig>`
.. |LinFrameResponse| replace:: :cpp:class:`LinFrameResponse<SilKit::Services::Lin::LinFrameResponse>`
.. |LinFrame| replace:: :cpp:class:`LinFrame<SilKit::Services::Lin::LinFrame>`

.. |LinControllerMode| replace:: :cpp:enum:`LinControllerMode<SilKit::Services::Lin::LinControllerMode>`
.. |LinControllerMode_Master| replace:: :cpp:enumerator:`LinControllerMode::Master<SilKit::Services::Lin::Master>`
.. |LinControllerMode_Slave| replace:: :cpp:enumerator:`LinControllerMode::Slave<SilKit::Services::Lin::Slave>`

.. |LinFrameResponseType_MasterResponse| replace:: :cpp:enumerator:`LinFrameResponseType::MasterResponse<SilKit::Services::Lin::LinFrameResponseType::MasterResponse>`
.. |LinFrameResponseType_SlaveResponse| replace:: :cpp:enumerator:`LinFrameResponseType::SlaveResponse<SilKit::Services::Lin::LinFrameResponseType::SlaveResponse>`
.. |LinFrameResponseType_SlaveToSlave| replace:: :cpp:enumerator:`LinFrameResponseType::SlaveToSlave<SilKit::Services::Lin::LinFrameResponseType::SlaveToSlave>`

.. |LinFrameResponseMode_Rx| replace:: :cpp:enumerator:`LinFrameResponseMode::Rx<SilKit::Services::Lin::LinFrameResponseMode::Rx>`
.. |LinFrameResponseMode_Tx| replace:: :cpp:enumerator:`LinFrameResponseMode::TxUnconditional<SilKit::Services::Lin::LinFrameResponseMode::TxUnconditional>`

.. |LinChecksumModel| replace:: :cpp:enum:`LinFrameStatus::LinChecksumModel<SilKit::Services::Lin::LinChecksumModel>`
.. |LinChecksumModel_Undefined| replace:: :cpp:enumerator:`LinFrameStatus::LinChecksumModel::Unknown<SilKit::Services::Lin::LinChecksumModel::Unknown>`

.. |LinDataLength| replace:: :cpp:type:`LinDataLength<SilKit::Services::Lin::LinDataLength>`
.. |LinDataLengthUnknown| replace:: :cpp:var:`LinDataLengthUnknown<SilKit::Services::Lin::LinDataLengthUnknown>`

.. |LinFrameStatus| replace:: :cpp:enum:`LinFrameStatus<SilKit::Services::Lin::LinFrameStatus>`
.. |LinFrameStatus_LIN_RX_OK| replace:: :cpp:enumerator:`LinFrameStatus::LIN_RX_OK<SilKit::Services::Lin::LIN_RX_OK>`
.. |LinFrameStatus_LIN_TX_OK| replace:: :cpp:enumerator:`LinFrameStatus::LIN_TX_OK<SilKit::Services::Lin::LIN_TX_OK>`
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
A LIN controller is created by calling |CreateLinController| given a controller and network name::

  auto* linMasterController = participant->CreateLinController("LinMaster", "LIN1");

LIN controllers will only communicate within the same network.

.. _sec:lin-initialization:

Initialization
~~~~~~~~~~~~~~

Before the LIN Controller can be used, it must be initialized. The initialization is performed by setting up a
|LinControllerConfig| and passing it to |Init|. The |LinControllerMode| must be set to either 
|LinControllerMode_Master| or |LinControllerMode_Slave| and the baud rate must be specified. Further, the 
|LinControllerConfig| provides the configuration on which LIN IDs the controller will receive 
(|LinFrameResponseMode_Rx|) or respond to (|LinFrameResponseMode_Tx|) frames.

The following example configures a LIN controller as a LIN slave with a baud rate of 20'000 baud. Furthermore, LIN ID 
0x11 is configured for transmission::

    LinFrameResponse response;
    response.frame.id = 0x11;
    response.frame.checksumModel = LinChecksumModel::Enhanced;
    response.frame.dataLength = 8;
    response.frame.data = {1, 2, 3, 4, 5, 6, 7, 8};
    response.responseMode = LinFrameResponseMode::TxUnconditional;

    LinControllerConfig slaveConfig;
    slaveConfig.controllerMode = LinControllerMode::Slave;
    slaveConfig.baudRate = 20000;
    slaveConfig.frameResponses.push_back(response);

    linController->Init(slaveConfig);

Note that |Init| must only be called once. A second call of |Init| or operations on an uninitialized LIN node will 
result in an exception. For the configuration, also keep in mind that the LIN protocol allows multiple nodes to be 
configured for reception, but only one node to provide a response. This restriction is not evaluated upon |Init|,
but only during operation.

Extending the configuration during operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Initialized LIN nodes can use |SetFrameResponse| to configure a response or reception during operation.
|SetFrameResponse| is useful for cases where a LIN slave is not aware of its full configuration upon initialization.
Reconfiguration of already used ID on the particular node are discarded. If used on a slave, the new configuration 
will become active once the master receives the updated configuration.

For a LIN master, the AUTOSAR API |SendFrame| will also extend the configuration and can be used for IDs not explicitly
defined in |Init|. If used with |LinFrameResponseType_MasterResponse|, the master will configure 
|LinFrameResponseMode_Tx|, if used with |LinFrameResponseType_SlaveResponse|, the master will configure 
|LinFrameResponseMode_Rx| on the given ID. In case of |LinFrameResponseType_SlaveToSlave|, no reconfiguration takes place,
but the master will receive a call to its |FrameStatusHandler| with |LinFrameStatus_LIN_TX_OK|, confirming the 
initiation of the slave-to-slave communication.

Initiating LIN Transmissions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Data is transferred in the form of a |LinFrame|, reception and acknowledgement is handled in the |FrameStatusHandler|.
A LIN master can initiate the transmission of a frame using the AUTOSAR API |SendFrame| or the non-AUTOSAR API 
|SendFrameHeader|. If a LIN slave provides the response, |SendFrame| requires that a corresponding frame 
response was configured before at a LIN slave using |Init| or |SetFrameResponse|.

If using |SendFrame| with |LinFrameResponseType_MasterResponse|, the response doesn't have to be preconfigured and the 
payload provided in the frame parameter of |SendFrame| will be used. If the LIN master uses the non-AUTOSAR API 
|SendFrameHeader|, a LIN node has to be configured with |LinFrameResponseMode_Tx| on that ID, possibly the master 
itself.

In all cases except |SendFrame| with |LinFrameResponseType_MasterResponse|, the responding node will receive the 
header and complete the transmission using the payload of its current transmit buffer. The buffer can be updated
with |UpdateTxBuffer| on the node that is configured with |LinFrameResponseMode_Tx| for the corresponding ID.

Sending Data
____________

The transmission of a frame can be initiated using |SendFrame| or |SendFrameHeader|:

1. Using |SendFrame| with |LinFrameResponseType_MasterResponse|, the frame can be sent in one call
   and the master provides both header and response. A |LinFrame| must be setup with the LIN ID,
   data, data length and the desired checksum model::

        // Prepare a frame with id 0x10 for transmission
        LinFrame masterFrame;
        masterFrame.id = 0x10;
        masterFrame.dataLength = 8;
        masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
        masterFrame.checksumModel = LinChecksumModel::Enhanced;

        // initiate the frame transmission using the AUTOSAR interface
        master->SendFrame(masterFrame, LinFrameResponseType::MasterResponse);

2. When using |SendFrame| with |LinFrameResponseType_SlaveResponse| or |LinFrameResponseType_SlaveToSlave|, 
   a slave has to be preconfigured with |LinFrameResponseMode_Tx| on that ID. With these response types, only the 
   ID of the |LinFrame| used in |SendFrame| is taken into account. The actual payload and frame settings are provided
   by the Tx buffer of the responing slave. The following example assumes that a slave is configured as seen in 
   :ref:`Initialization<sec:lin-initialization>`::

        // The slave is configured to respond on ID 0x11
        LinFrame slaveResponseFrame;
        slaveResponseFrame.id = 0x11;

        // initiate the frame transmission using the AUTOSAR interface
        master->SendFrame(slaveResponseFrame, LinFrameResponseType::SlaveResponse);
       
Using |SendFrame| with |LinFrameResponseType_SlaveResponse| assumes that the master is interested in the response
and will configure itself for reception (|LinFrameResponseMode_Rx|).

3. When using |SendFrameHeader|, the transmission is initiated by sending the header. The node (either master or slave) configured with
   |LinFrameResponseMode_Tx| will provide the response. The actual payload and frame settings are 
   provided by the Tx buffer of the responing LIN node. The following example also assumes that a slave is 
   configured as seen in :ref:`Initialization<sec:lin-initialization>`::

        // Slave:
        LinFrame updatedSlaveResponse;
        updatedSlaveResponse.id = 0x11;
        updatedSlaveResponse.dataLength = 8;
        updatedSlaveResponse.data = {'S', 'L', 'A', 'V', 'E', 1, 2, 3};
        updatedSlaveResponse.checksumModel = LinChecksumModel::Enhanced;
        slave->UpdateTxBuffer(updatedSlaveResponse);

        // Master:
        // Transmit the frame header, the response will be provided by the slave.
        master->SendFrameHeader(0x11);

Transmission acknowledgement
____________________________

To be notified for the success or failure of the transmission, a |FrameStatusHandler| should be registered using 
|AddFrameStatusHandler|::
  
  // Register FrameStatusHandler to receive data from the LIN slave
  auto frameStatusHandler =
      [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
  master->AddFrameStatusHandler(frameStatusHandler);

A successful transmission is confirmed via the registered callback, for example::

  frameStatusHandler(master, frameStatusEvent);
  // With:
  // frameStatusEvent.timestamp: timeEndOfFrame;
  // frameStatusEvent.frame: masterFrame;
  // frameStatusEvent.status: LinFrameStatus::LIN_TX_OK;

If multiple controllers have configured |LinFrameResponseMode_Tx| on the same LIN ID, a collision occurs on the bus,
which is indicated by |LinFrameStatus_LIN_TX_ERROR|.

Receiving data from a slave
___________________________

Beside transmission acknowledgements, the |FrameStatusHandler| is also used for reception. To receive data, the 
|FrameStatusHandler| must be registered using |AddFrameStatusHandler|, which is called by the LIN controller when 
a frame is received and the LIN node is configured for reception with |LinFrameResponseMode_Rx| on that LIN ID::

    // Register FrameStatusHandler to receive data from the LIN slave
    auto frameStatusHandler =
        [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
    master->AddFrameStatusHandler(frameStatusHandler);

The incoming |LinFrameStatusEvent| holds a |LinFrameStatus| indicating the success (|LinFrameStatus_LIN_RX_OK|) or 
failure (|LinFrameStatus_LIN_RX_ERROR|) of the transmission. If a LIN slave has previously setup a matching 
|LinFrameResponse| for transmission, the registered |FrameStatusHandler| will deliver a |LinFrameStatusEvent| as 
follows::

    frameStatusHandler(master, frameStatusEvent);
    // With:
    // frameStatusEvent.timestamp: timeEndOfFrame;
    // frameStatusEvent.frame: slaveFrame;
    // frameStatusEvent.status: LinFrameStatus::LIN_RX_OK;

If more than one slave provided a response, the master will receive a |LinFrameStatus_LIN_RX_ERROR|, the slaves will 
see |LinFrameStatus_LIN_TX_ERROR|.

Data length and checksum model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A |LinDataLength| and |LinChecksumModel| can be provided for a given ID when configuring a reception or initiating a 
transmission. A frame will arrive with |LinFrameStatus_LIN_RX_ERROR| if there is a mismatch between configured and 
received data length or checksum model. However, a LIN node configured for reception might not know beforehand about the
data length or checksum model provided in the response. In this case, the reception can be configured with the 
wildcards |LinDataLengthUnknown| or |LinChecksumModel_Undefined| in the respective paramters of the |LinFrame| and the 
data length or checksum model provided by the sender is used in the |FrameStatusHandler|.

Managing the event handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameStatusHandler|
- |RemoveGoToSleepHandler|
- |RemoveWakeupHandler|


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

.. doxygentypedef:: SilKit::Services::Lin::LinId
.. doxygenenum:: SilKit::Services::Lin::LinChecksumModel
.. doxygentypedef:: SilKit::Services::Lin::LinDataLength
.. doxygenvariable:: SilKit::Services::Lin::LinDataLengthUnknown
.. doxygenenum:: SilKit::Services::Lin::LinFrameResponseType
.. doxygenenum:: SilKit::Services::Lin::LinFrameResponseMode
.. doxygenenum:: SilKit::Services::Lin::LinFrameStatus
.. doxygenenum:: SilKit::Services::Lin::LinControllerMode
.. doxygentypedef:: SilKit::Services::Lin::LinBaudRate
.. doxygenenum:: SilKit::Services::Lin::LinControllerStatus

Usage Examples
--------------

This section contains more complex examples that show the interaction of two or more LIN controllers. Although the LIN 
controllers would typically belong to different participants and reside in different processes, their interaction is
shown here sequentially to demonstrate cause and effect.

Assumptions:

- Variables ``master``, ``slave``, ``slave1``, and ``slave2`` are of type |ILinController|.
- Variable ``timeEndOfFrame`` indicates the end of frame time stamp when using the detailed simulation. Otherwise, the value of 
  ``timeEndOfFrame`` is undefined.
- ``UseAutosarInterface`` is a boolean variable that indicates whether to use the AUTOSAR API or the non-AUTOSAR API.
  It will most likely not be used in practice, and only serves to show the various uses of the API.
  
Successful Transmission from Master to slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN master to a LIN slave. The transmission must be initiated by 
the master.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_OK.cpp
   :language: cpp
   
Successful Transmission from slave to Master
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN slave to a LIN master. The transmission must be initiated by 
the master.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_OK.cpp
   :language: cpp

Successful Transmission from slave to slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how data is transferred from one LIN slave to another one. The data transfer must be initiated by a 
LIN master.

.. literalinclude::
   examples/lin/Slave_to_Slave_LIN_TX_OK.cpp
   :language: cpp
              
Erroneous Transmission from Master to Slave - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master attempts to send a Frame while there is slave that has configured a Tx 
response for the same LIN ID.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_ERROR.cpp
   :language: cpp
   
Erroneous Transmission from Slave to Master - No Response
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission and no slave has configured a Tx response for 
this LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_NO_RESPONSE.cpp
   :language: cpp


Erroneous Transmission from Slave to Master - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission where multiple slaves have configured Tx 
responses for the same LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_ERROR.cpp
   :language: cpp

Go To Sleep
~~~~~~~~~~~

This example shows how to use |GoToSleep| and when the controller will switch from operational to sleep mode.

.. literalinclude::
   examples/lin/Go_To_Sleep.cpp
   :language: cpp

.. admonition:: Note
    
    The |GoToSleepHandler| is triggered even without configuring ID ``0x3C`` for reception. However, the 
    |FrameStatusHandler| for slaves is only called if ID ``0x3C`` is configured for reception. The master initiated
    the sleep mode, it's |FrameStatusHandler| is called with |LinFrameStatus_LIN_TX_OK|.

Wake Up
~~~~~~~

This example shows how to |Wakeup| a LIN bus. The example assumes that both master and slave are currently in sleep mode.
I.e., the situation corresponds to the end of the previous example.

.. literalinclude::
   examples/lin/Wake_Up.cpp
   :language: cpp

Aggregated view of responding LIN slaves (experimental)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how the |LinSlaveConfigurationHandler| provides direct access to the |LinFrameResponse| configuration of 
all slaves. This can be used by a LIN master to predict if a slave response will be provided prior to the use of |SendFrame|
or |SendFrameHeader|. It is primarily intended for diagnostic purposes and not required for regular operation of a LIN
controller. The calls `AddLinSlaveConfigurationHandler`, `RemoveLinSlaveConfigurationHandler` and `GetSlaveConfiguration` 
reside in the `SilKit::Experimental::Services::Lin` namespace and might be changed or removed in future versions.

.. literalinclude::
   examples/lin/LinSlaveConfigurationHandler.cpp
   :language: cpp

The experimental API is defined as follows:

.. doxygenfunction:: SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler)
.. doxygenfunction:: SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* linController, SilKit::Util::HandlerId handlerId)
.. doxygenfunction:: SilKit::Experimental::Services::Lin::GetSlaveConfiguration(SilKit::Services::Lin::ILinController* linController)
