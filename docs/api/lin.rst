.. _sec:lin:

===================
LIN Service API
===================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp
   
Using the LIN Controller
-------------------------

Initialization
~~~~~~~~~~~~~~~~~~~~

Before the LIN Controller can be used, it must be initialized. The
initialization is performed by setting up a
:cpp:class:`ControllerConfig<ib::sim::lin::ControllerConfig>` and passing it to
:cpp:func:`ILinController::Init()<ib::sim::lin::ILinController::Init>`.

At a minimum, the controllerMode must be set to either ControllerMode::Master or
ControllerMode::Slave. If the VIBE NetworkSimulator is used, also a baud rate
must be specified. In addition, the
:cpp:class:`ControllerConfig<ib::sim::lin::ControllerConfig>` allows providing
an initial set of :cpp:class:`FrameResponses<ib::sim::lin::FrameResponse>`,
which is particularly useful for LIN slaves.

The following example configures a LIN controller as a LIN slave with a baud
rate of 20'000 baud. Furthermore, LIN ID 0x11 is configured for transmission::

    Frame slaveFrame;
    slaveFrame.id = 0x11;
    slaveFrame.checksumModel = ChecksumModel::Enhanced;
    slaveFrame.dataLength = 8;
    slaveFrame.data = {1,2,3,4,5,6,7,8};
    
    ControllerConfig slaveConfig;
    slaveConfig.controllerMode = ControllerMode::Slave;
    slaveConfig.baudRate = 20000;
    slaveConfig.frameResponses.push_back(slaveFrame);
    
    linController->Init(slaveConfig);

Note that :cpp:func:`ILinController::Init()<ib::sim::lin::ILinController::Init>`
should be called in the InitHandler of a ParticipantController.


Initiating LIN Transmissions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Data is transfered in the form of a LIN
:cpp:class:`Frame<ib::sim::lin::Frame>`. A LIN master can initiate
the transmission of a frame using the AUTOSAR API
(:cpp:func:`ILinController::SendFrame()<ib::sim::lin::ILinController::SendFrame>`)
or using the low-level, non-AUTOSAR API
(:cpp:func:`ILinController::SendFrameHeader()<ib::sim::lin::ILinController::SendFrameHeader>`)
which requires that a corresponding frame response was configured
befor at any LIN controller
(:cpp:func:`ILinController::SetFrameResponse()<ib::sim::lin::ILinController::SetFrameResponse>`).

Sending Data to Slaves
________________________________________

To send data, a LIN :cpp:class:`Frame<ib::sim::lin::Frame>` must be setup with the LIN ID,
the data to be transmitted, and the desired checksum model::

  // Prepare a frame with id 0x10 for transmission
  Frame masterFrame;
  masterFrame.id = 0x10;
  masterFrame.dataLength = 8;
  masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
  masterFrame.checksumModel = ChecksumModel::Enhanced;

To be notified for the success or failure of the transmission, a FrameStatusHandler should
be registered::
  
  // Register FrameStatusHandler to receive data from the LIN slave
  auto frameStatusHandler =
      [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
  master->RegisterFrameStatusHandler(frameStatusHandler);

The transmission of the frame can be initiated using either the AUTOSAR interface or the
low-level, non-AUTOSAR interface:

    1. Using the AUTOSAR interface, the frame can be sent in one call with
       FrameResponseType::MasterResponse indicating that the master provides both header
       and response::

         // send the frame using the AUTOSAR interface
         master->SendFrame(masterFrame, FrameResponseType::MasterResponse);

    2. Using the low-level interface, the frame is sent in two steps. First a
       FrameResponse is configured with TxUnconditional, and then a LIN header with the
       corresponding id is sent::

         // Setup the master response
         master->SetFrameResponse(masterFrame, FrameResponseMode::TxUnconditional);

         // Transmit the frame header, the response will be transmitted automatically.
         master->SendFrameHeader(0x10);


A successful transmission is confirmed via the registered callback, e.g.::

  frameStatusHandler(master, masterFrame, FrameStatus::LIN_TX_OK, timeEndOfFrame);


.. admonition:: Note

    If another controller also has ID 0x11 configured for transmission, a collision occurs
    on the bus, which is indicated by FrameStatus::LIN_TX_ERROR.

.. admonition:: Note

    The end of frame timestamp is only valid when using the VIBE
    NetworkSimulator or when provided to
    :cpp:func:`ILinController::SendFrame()<void ib::sim::lin::ILinController::SendFrame(Frame, FrameResponseType, std::chrono::nanoseconds)>`
    or
    :cpp:func:`ILinController::SendFrameHeader()<void ib::sim::lin::ILinController::SendFrameHeader(LinIdT, std::chrono::nanoseconds)>`.


Receiving Data from a Slave
__________________________________________

To receive data from a LIN slave, a FrameStatusHandler must be registered, which is called by the LIN
controller whenever a frame was received::

    // Register FrameStatusHandler to receive data from the LIN slave
    auto frameStatusHandler =
        [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
    master->RegisterFrameStatusHandler(frameStatusHandler);

To initiate the data transfer from slave to master, the master must provide the LIN ID as
well as the expected data length and checksum model::

    // Prepare to receive a frame with id 0x11
    Frame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = ChecksumModel::Enhanced;
    frameRequest.dataLength = 8;

Again, the transmission can be initiated using either the AUTOSAR interface or the
low-level, non-AUTOSAR interface.

    1. Using the AUTOSAR interface, the frame can be sent in one call. The
       FrameResponseType::SlaveResponse indicates that the master only provides the frame
       header and expects a slave to provide the response::
    
        // Initiate the transmission
        master->SendFrame(frameRequest, FrameResponseType::SlaveResponse);

    2. Alternatively, the transmission can be initiated using the low-level interface. This
       requires first to configure a FrameResponse with FrameResponseMode::Rx, and then
       sending a LIN header with the corresponding id::

        // Configure LIN ID 0x11 for reception
        master->SetFrameResponse(frameRequest, FrameResponseMode::Rx);
    
        // Initiate the transmission, a FrameResponse must be setup by a LIN slave
        master->SendFrameHeader(0x11);

    
The end of the transmission will be confirmed by calling the registered FrameStatusHandler
(here, frameStatusHandler) with FrameStatus indicating the success or failure of
the transmission.

If a LIN slave has previously setup a matching FrameResponse for transmission, the
registered frameStatusHandler will be called as follows::

    frameStatusHandler(master, slaveFrame, FrameStatus::LIN_RX_OK, timeEndOfFrame);

.. admonition:: Note

    If the frame response provided by the LIN frame does not match both expected
    dataLength and checksumModel, or if more than one slave provided a response, the
    FrameStatus::LIN_RX_ERROR will be used.

    If no LIN slave provides a frame response, the FrameStatus::LIN_RX_NO_RESPONSE will be
    used.

Message Tracing
~~~~~~~~~~~~~~~
The LinController supports message tracing in MDF4 format.
This is provided by the :ref:`VIBE MDF4Tracing<mdf4tracing>` extension.
Refer to the :ref:`sec:cfg-participant-tracing` configuration section for usage instructions.

     
API and Data Type Reference
--------------------------------------------------
LIN Controller API
~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::sim::lin::ILinController
   :members:

Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::lin::Frame
   :members:
.. doxygenstruct:: ib::sim::lin::FrameResponse
   :members:
.. doxygenstruct:: ib::sim::lin::ControllerConfig
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
----------------------------------------------------

This section contains more complex examples that show the interaction of two or
more LIN controllers. Although the LIN controllers would typically belong to
different participants and reside in different processes, their interaction is
shown here sequentially to demonstrate cause and effect.

Assumptions:

- *master*, *slave*, *slave1*, and *slave2* are of type
  :cpp:class:`ILinController*<ib::sim::lin::ILinController>`.
- *timeEndOfFrame* indicates the end of frame time stamp when using the VIBE
  NetworkSimulator. A time stamp can also be provided when initiating a transmission through
  :cpp:func:`ILinController::SendFrame()<void ib::sim::lin::ILinController::SendFrame(Frame, FrameResponseType, std::chrono::nanoseconds)>`
  or
  :cpp:func:`ILinController::SendFrameHeader()<void ib::sim::lin::ILinController::SendFrameHeader(LinIdT, std::chrono::nanoseconds)>`.
  Otherwise the value of *timeEndofFrame* is undefined.
- *UseAutosarInterface* is a boolean variable that indicates whether
  to use the AUTOSAR API or the non-AUTOSAR API. It will most likely
  not be used in practice and it merely intended to show the different
  usages of the API.

  
Successful Transmission from Master to Slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN master to a LIN
slave. The transmission must be initiated by the master.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_OK.cpp
   :language: cpp
   
Successful Transmission from Slave to Master
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from a LIN Slave to a LIN
master. The transmission must be initiated by the master.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_OK.cpp
   :language: cpp

Successful Transmission from Slave to Slave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how data is transferred from one LIN slave to another
one. The data transfer must be initiated by a LIN master.

.. literalinclude::
   examples/lin/Slave_to_Slave_LIN_TX_OK.cpp
   :language: cpp
              
Erroneous Transmission from Master to Slave - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master atempts to send a Frame while
there is slave that has configured a TX response for the same LIN ID.

.. literalinclude::
   examples/lin/Master_to_Slave_LIN_TX_ERROR.cpp
   :language: cpp
   
Erroneous Transmission from Slave to Master - No Response
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission and no
slave hasconfigured a TX response for the this LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_NO_RESPONSE.cpp
   :language: cpp

.. admonition:: Note

   For simplicity, this example only uses the AUTOSAR interface, the
   behavior and error code would be the same for the non-AUTOSAR
   interface.

Erroneous Transmission from Slave to Master - Multiple Responses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows what happens when a master initiates a transmission where
multiple slaves have configured TX responses for the same LIN ID.

.. literalinclude::
   examples/lin/Slave_to_Master_LIN_RX_ERROR.cpp
   :language: cpp

Go To Sleep
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how to initiate a Go To Sleep and when the controller switch
from operational to sleep mode.

.. literalinclude::
   examples/lin/Go_To_Sleep.cpp
   :language: cpp

.. admonition:: Note
    
    The GoToSleepHandler is triggered even without configuring
    ID 0x3C for reception. However, the FrameStatusHandler is
    only called if ID 0x3C is configured for reception.

Wake Up
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how to wake up a LIN bus. The example assumes that both
master and slave are currently in sleep mode. I.e., the situation corresponds to
the end of the previous example.

.. literalinclude::
   examples/lin/Wake_Up.cpp
   :language: cpp

FrameResponseUpdateHandler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows how the FrameResponseUpdate handler provides direct access to
the FrameResponse configuration of all slaves. It is primarily intended for
diagnostic purposes and not required for regular operation of a LIN controller.

.. literalinclude::
   examples/lin/FrameResponseUpdateHandler.cpp
   :language: cpp



              
The new LIN Controller API
----------------------------------------------------

This version of the Vector Integration Bus introduces a redesigned LIN API with a clear
new interface. The main reasons for the redesign were inconsistencies in the usage and an
incoherent interface. E.g., callbacks for successful transmission and reception varied
depending on the configuration as a slave or a master, there was no clear API to initiate
slave-to-slave data transfers or to initiate only a header transmission without needing to
provide a payload.

To overcome these issues, we decided to provide a LIN controller API that follows the
AUTOSAR LIN driver API, a widely adopted standard. Further more, we added a few extensions
for a more low level send interface.

While, on the first glance, the API seems to be very different, it should be fairly easy
to adapt to the changes. This section lists the major changes and the documentation for
the new API provides examples on how to use it.


Major Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compared to the previous design, the new LIN Controller API provides the following
benefits:

  - A uniform user experience for trivial simulation and VIBE simulation
  - A flexible API to initiate data transfers:
      + An AUTOSAR compatible API with
        :cpp:func:`ILinController::SendFrame()<ib::sim::lin::ILinController::SendFrame>`
      + A low-level, non-AUTOSAR API with
        :cpp:func:`ILinController::SetFrameResponse()<ib::sim::lin::ILinController::SetFrameResponse>`
        and
        :cpp:func:`ILinController::SendFrameHeader()<ib::sim::lin::ILinController::SendFrameHeader>`
  - An AUTOSAR compatible, unified callback for all LIN transmissions,
    i.e., for frame reception and TX acknowledges for both LIN
    mastersand LIN slaves:
    :cpp:func:`ILinController::RegisterFrameStatusHandler()<ib::sim::lin::ILinController::RegisterFrameStatusHandler>`,
    which uses a revised :cpp:enum:`FrameStatus<ib::sim::lin::FrameStatus>` type that now uses AUTOSAR identifiers.
  - An AUTOSAR compatible interface for GoToSleep and Wake-Up
      * :cpp:func:`ILinController::GoToSleep<ib::sim::lin::ILinController::GoToSleep>`
      * :cpp:func:`ILinController::GoToSleepInternal<ib::sim::lin::ILinController::GoToSleepInternal>`
      * :cpp:func:`ILinController::Wakeup<ib::sim::lin::ILinController::Wakeup>`
      * :cpp:func:`ILinController::WakeupInternal<ib::sim::lin::ILinController::WakeupInternal>`
  - Initialization using a single method with a single config struct:
    :cpp:func:`ILinController::Init()<ib::sim::lin::ILinController::Init>`
  - A new callback to get direct access to all FrameResponse updates from connected slaves:
    :cpp:func:`ILinController::RegisterFrameResponseUpdateHandler<ib::sim::lin::ILinController::RegisterFrameResponseUpdateHandler>`


.. _sec:datatype-changes:

Overview of Data Type Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  - :cpp:type:`LinId` has been renamed to :cpp:type:`LinIdT<ib::sim::lin::LinIdT>`.
  - The old enum :cpp:enum:`ControllerMode<ib::sim::lin::ControllerMode_>`,
    which combined master, slave, and sleep states, has been split into
    :cpp:enum:`ControllerMode<ib::sim::lin::ControllerMode>` (indicating Master
    or Slave operation) and
    :cpp:enum:`ControllerStatus<ib::sim::lin::ControllerStatus>` (indicating
    operational and sleep states).
  - :cpp:enum:`ChecksumModel<ib::sim::lin::ChecksumModel>` is unchanged.
  - :cpp:enum:`ResponseMode` has been renamed to
    :cpp:enum:`FrameResponseMode<ib::sim::lin::FrameResponseMode>`; the enumeration
    values habe been left unchanged.
  - The old :cpp:class:`Payload` data type, which combined a fixed size
    std::array<uint8_t> and a size value, has been removed. The
    :cpp:class:`Frame<ib::sim::lin::Frame>` now provides dataLength and data
    fields directly.
  - :cpp:enum:`MessageStatus` has been renamed to
    :cpp:enum:`FrameStatus<ib::sim::lin::FrameStatus>` and the enumeration names have been
    changed to AUTOSAR naming. For details, check the following conversion table:

    ==================== ====================
    MessageStatus (old)  FrameStatus (new) 
    ==================== ====================
    TxSuccess            LIN_TX_OK
    RxSuccess            LIN_RX_OK
    TxResponseError      LIN_TX_ERROR
    RxResponseError      LIN_RX_ERROR
    RxNoResponse         LIN_RX_NO_RESPONSE
    HeaderError          LIN_TX_HEADER_ERROR
    Canceled             NOT_OK
    Busy                 LIN_TX_BUSY
    \-                   LIN_RX_BUSY
    ==================== ====================
