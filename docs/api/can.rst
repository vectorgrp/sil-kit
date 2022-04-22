===================
CAN Service API
===================

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the CAN Controller
-------------------------

The CAN Service API provides a CAN bus abstraction through :cpp:class:`CanControllers<ib::sim::can::ICanController>`.
A CAN controller can be created by calling :cpp:func:`IParticiant::CreateCanController()<ib::mw::IParticipant::CreateCanController>`.

Initialization
~~~~~~~~~~~~~~~~~~~~

For a detailed simulation with the :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`, the baud rate of a CAN controller needs to be set by passing it to
:cpp:func:`ICanController::SetBaudRate()<ib::sim::can::ICanController::SetBaudRate>` before using it. 
Furthermore, it has to be started explicitly by calling 
:cpp:func:`ICanController::Start()<ib::sim::can::ICanController::Start>`. 
These functions can but do not have to be called in a simple functional simulation without :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`.

The following example configures a CAN controller with a baud rate of 10'000 baud
for regular CAN messages and a baud rate of 1'000'000 baud for CAN FD messages.
Then, the controller is started::

    canController->SetBaudRate(10000, 1000000);
    canController->Start();

.. admonition:: Note

   Both :cpp:func:`ICanController::SetBaudRate()<ib::sim::can::ICanController::SetBaudRate>`
   and :cpp:func:`ICanController::Start()<ib::sim::can::ICanController::Start>`
   should not be called earlier than in the participant controller's
   :cpp:func:`init handler<ib::mw::synd::IParticipantController::SetInitHandler()>`. Otherwise,
   it is not guaranteed that all participants are already connected, which can cause the call
   to have no effect.


Sending CAN Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Data is transfered in the form of a :cpp:class:`CanMessage<ib::sim::can::CanMessage>`.
To send a :cpp:class:`CanMessage<ib::sim::can::CanMessage>`, it must be setup with
a CAN ID and the data to be transmitted. In VIBE simulation
the :cpp:class:`CanMessage::CanReceiveFlags<ib::sim::can::CanMessage::CanReceiveFlags>` are also relevant::

  // Prepare a CAN message with id 0x17
  CanMessage canMessage;
  canMessage.timestamp = now;
  canMessage.canId = 0x17;
  canMessage.ide = 0;  // Identifier Extension
  canMessage.rtr = 0;  // Remote Transmission Request
  canMessage.fdf = 0;  // FD Format Indicator
  canMessage.brs = 1;  // Bit Rate Switch  (for FD Format only)
  canMessage.esi = 0;  // Error State indicator (for FD Format only)
  canMessage.dataField = {'d', 'a', 't', 'a', 0, 1, 2, 3};

  canController.SendMessage(canMessage);

To be notified of the success or failure of the transmission, a MessageStatusHandler should
be registered::
  
  // Register MessageStatusHandler to receive CAN acknowledges from other CAN controller.
  auto messageStatusHandler =
      [](ICanController*, const can::CanTransmitAcknowledge& ack) {};
  canController->RegisterTransmitStatusHandler(messageStatusHandler);

The :cpp:class:`CanTransmitAcknowledge<ib::sim::can::CanTransmitAcknowledge>` received in the MessageStatusHandler will always have the value
:cpp:enumerator:`Transmitted<ib::sim::can::Transmitted>` in a simple simulation without VIBE NetworkSimulator,
which indicates a successful transmission. If the VIBE NetworkSimulator is used, it is
possible that the transmit queue overflows causing the handler to be called with
:cpp:enumerator:`TransmitQueueFull<ib::sim::can::TransmitQueueFull>` signaling a transmission failure.


Receiving CAN Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive data from other CAN controllers, a ReceiveMessageHandler must be
registered, which is called by the CAN controller whenever a CAN message is received::

  // Register ReceiveMessageHandler to receive CAN messages from other CAN controller.
  auto receiveMessageHandler =
      [](ICanController*, const CanMessage&) {};
  canController->RegisterReceiveMessageHandler(receiveMessageHandler);

An optional second parameter allows to specify the direction (TX, RX, TX/RX) of the CanFrames to be received. 
By default, only CanFrames of the direction TX are handled.

Message Tracing
~~~~~~~~~~~~~~~

.. admonition:: Note

  Currently the Message Tracing functionality is not available, but it will be reintegrated in the future.

The CanController supports message tracing in MDF4 format.
This is provided by the :ref:`VIBE MDF4Tracing<mdf4tracing>` extension.
Refer to the :ref:`sec:cfg-participant-tracing` configuration section for usage instructions.

API and Data Type Reference
--------------------------------------------------
CAN Controller API
~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::sim::can::ICanController
   :members:

Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::can::CanMessage
   :members:
.. doxygenstruct:: ib::sim::can::CanTransmitAcknowledge
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib::sim::can::CanTxId
.. doxygenenum:: ib::sim::can::CanControllerState
.. doxygenenum:: ib::sim::can::CanErrorState
.. doxygenenum:: ib::sim::can::CanTransmitStatus


Usage Examples
----------------------------------------------------

This section contains complete examples that show the usage of the CAN controller
and the interaction of two or more controllers. Although the CAN controllers would
typically belong to different participants and reside in different processes,
their interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- *canReceiver*, *canSender* are of type
  :cpp:class:`ICanController*<ib::sim::can::ICanController>`.
- All CAN controllers are connected on the same CAN bus.

Simple CAN Sender / Receiver Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one CAN controller
to another CAN controller connected on the same CAN bus.

.. literalinclude::
   examples/can/CAN_Sender_Receiver.cpp
   :language: cpp
