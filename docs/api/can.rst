===================
CAN Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>`
.. |CreateCanController| replace:: :cpp:func:`CreateCanController<ib::mw::IParticipant::CreateCanController()>`
.. |ICanController| replace:: :cpp:class:`ICanController<ib::sim::can::ICanController>`

.. |SendFrame| replace:: :cpp:func:`SendFrame()<ib::sim::can::ICanController::SendFrame>`
.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<ib::sim::can::ICanController::AddFrameTransmitHandler>`
.. |AddStateChangeHandler| replace:: :cpp:func:`AddStateChangeHandler()<ib::sim::can::ICanController::AddStateChangeHandler>`
.. |AddErrorStateChangeHandler| replace:: :cpp:func:`AddErrorStateChangeHandler()<ib::sim::can::ICanController::AddErrorStateChangeHandler>`
.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<ib::sim::can::ICanController::AddFrameHandler>`
.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<ib::sim::can::ICanController::RemoveFrameTransmitHandler>`
.. |RemoveStateChangeHandler| replace:: :cpp:func:`RemoveStateChangeHandler()<ib::sim::can::ICanController::RemoveStateChangeHandler>`
.. |RemoveErrorStateChangeHandler| replace:: :cpp:func:`RemoveErrorStateChangeHandler()<ib::sim::can::ICanController::RemoveErrorStateChangeHandler>`
.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<ib::sim::can::ICanController::RemoveFrameHandler>`
.. |Start| replace:: :cpp:func:`Start()<ib::sim::can::ICanController::Start>`
.. |Stop| replace:: :cpp:func:`Stop()<ib::sim::can::ICanController::Stop>`
.. |Reset| replace:: :cpp:func:`Reset()<ib::sim::can::ICanController::Reset>`
.. |SetBaudRate| replace:: :cpp:func:`ICanController::SetBaudRate()<ib::sim::can::ICanController::SetBaudRate>`

.. |CanFrame| replace:: :cpp:class:`CanFrame<ib::sim::can::CanFrame>`
.. |CanFrameEvent| replace:: :cpp:class:`CanFrameEvent<ib::sim::can::CanFrameEvent>`
.. |CanFrameTransmitEvent| replace:: :cpp:class:`CanFrameTransmitEvent<ib::sim::can::CanFrameTransmitEvent>`
.. |CanStateChangeEvent| replace:: :cpp:class:`CanStateChangeEvent<ib::sim::can::CanStateChangeEvent>`
.. |CanErrorStateChangeEvent| replace:: :cpp:class:`CanErrorStateChangeEvent<ib::sim::can::CanErrorStateChangeEvent>`

.. |CanControllerState| replace:: :cpp:enum:`CanControllerState<ib::sim::can::CanControllerState>`
.. |CanErrorState| replace:: :cpp:enum:`CanErrorState<ib::sim::can::CanErrorState>`
.. |CanFrameFlags| replace:: :cpp:class:`CanFrame::CanFrameFlags<ib::sim::can::CanFrame::CanFrameFlags>`
.. |CanTransmitStatus| replace:: :cpp:enum:`CanTransmitStatus<ib::sim::can::CanTransmitStatus>`

.. |Transmitted| replace:: :cpp:enumerator:`CanTransmitStatus::Transmitted<ib::sim::can::Transmitted>`
.. |Canceled| replace:: :cpp:enumerator:`CanTransmitStatus::Canceled<ib::sim::can::Canceled>`
.. |TransmitQueueFull| replace:: :cpp:enumerator:`CanTransmitStatus::TransmitQueueFull<ib::sim::can::TransmitQueueFull>`
.. |DuplicatedTransmitId| replace:: :cpp:enumerator:`CanTransmitStatus::DuplicatedTransmitId<ib::sim::can::DuplicatedTransmitId>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<ib::sim::HandlerId>`

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the CAN Controller
-------------------------

The CAN Service API provides a CAN bus abstraction through the |ICanController| interface.
A CAN controller is created by calling |CreateCanController| given a controller name and (optional) network 
name::

  auto* canController = participant->CreateCanController("Can1", "CAN");

CAN controllers will only communicate within the same network. If no network name is provided, the controller name
will be used as the network name.

Sending CAN Frames
~~~~~~~~~~~~~~~~~~

Data is transfered in the form of a |CanFrame| and received as a |CanFrameEvent|. To send a |CanFrame|, it must be setup 
with a CAN ID and the data to be transmitted. In VIBE simulation the |CanFrameFlags| are also relevant::

  // Prepare a CAN message with id 0x17
  CanFrame canFrame;
  canFrame.canId = 3;
  canFrame.ide = 0;  // Identifier Extension
  canFrame.rtr = 0;  // Remote Transmission Request
  canFrame.fdf = 0;  // FD Format Indicator
  canFrame.brs = 1;  // Bit Rate Switch  (for FD Format only)
  canFrame.esi = 0;  // Error State indicator (for FD Format only)
  canFrame.dataField = {'d', 'a', 't', 'a', 0, 1, 2, 3};

  canController.SendFrame(canFrame);

Transmission acknowledgement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified of the success or failure of the transmission, a ``FrameTransmitHandler`` can be registered using
|AddFrameTransmitHandler|::

  auto frameTransmitHandler = [](ICanController*, const CanFrameTransmitEvent& frameTransmitEvent) 
  {
    // Handle frameTransmitEvent
  };
  canController->AddFrameTransmitHandler(frameTransmitHandler);

.. admonition:: Note

  In a simple simulation without the VIBE NetworkSimulator, the |CanTransmitStatus| of the |CanFrameTransmitEvent| will
  always be |Transmitted|. If the VIBE NetworkSimulator is used, it is possible that the transmit queue overflows 
  causing the handler to be called with |TransmitQueueFull| signaling a transmission failure.

Receiving CAN FrameEvents
~~~~~~~~~~~~~~~~~~~~~~~~~

A |CanFrame| is received as a |CanFrameEvent| consisting of a transmitId used to identify the acknowledge of the 
frame, a timestamp and the actual |CanFrame|. The handler is called whenever a |CanFrame| is received::

  auto frameHandler = [](ICanController*, const CanFrameEvent& frameEvent) 
  {
    // Handle frameEvent
  };
  canController->AddFrameHandler(frameHandler);

An optional second parameter of |AddFrameHandler| allows to specify the direction (TX, RX, TX/RX) of the CanFrames to be
received. By default, only frames of the direction RX are handled.

Receive state change events
~~~~~~~~~~~~~~~~~~~~~~~~~~~

State changes are only supported when using the VIBE NetworkSimulator. To receive changes of the |CanControllerState|,
a ``StateChangeHandler`` must be registered using |AddStateChangeHandler|::

  auto stateChangedHandler = [](ICanController*, const CanStateChangeEvent& stateChangeEvent) 
  {
    // Handle stateChangeEvent;
  };
  canController->AddStateChangeHandler(stateChangedHandler);

Similarly, changes in the |CanErrorState| can be tracked with |AddErrorStateChangeHandler|.

Initialization
~~~~~~~~~~~~~~

For a detailed simulation with the :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`, the baud rate of a CAN controller
needs to be set by passing it to |SetBaudRate| before using it. Furthermore, it has to be started explicitly by calling 
|Start|. Additional control commands for the detailed simulation are |Stop| and |Reset|. These functions can but do not 
have to be called in a simple functional simulation without :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`.

The following example configures a CAN controller with a baud rate of 10'000 baud for regular CAN messages and a baud 
rate of 1'000'000 baud for CAN FD messages. Then, the controller is started::

    canController->SetBaudRate(10000, 1000000);
    canController->Start();

.. admonition:: Note

   Both |SetBaudRate| and |Start|  should not be called earlier than in the participant controller's
   :cpp:func:`init handler<ib::mw::synd::IParticipantController::SetInitHandler()>`. Otherwise, it is not guaranteed 
   that all participants are already connected, which can cause the call to have no effect.

Managing the event handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameTransmitHandler|  
- |RemoveStateChangeHandler|
- |RemoveErrorStateChangeHandler|
- |RemoveFrameHandler|

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
.. doxygenstruct:: ib::sim::can::CanFrame
   :members:
.. doxygenstruct:: ib::sim::can::CanFrameEvent
   :members:
.. doxygenstruct:: ib::sim::can::CanFrameTransmitEvent
   :members:
.. doxygenstruct:: ib::sim::can::CanStateChangeEvent
   :members:
.. doxygenstruct:: ib::sim::can::CanErrorStateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: ib::sim::can::CanControllerState
.. doxygenenum:: ib::sim::can::CanErrorState
.. doxygenenum:: ib::sim::can::CanTransmitStatus
.. doxygentypedef:: ib::sim::can::CanTxId

Usage Examples
----------------------------------------------------

This section contains complete examples that show the usage of the CAN controller and the interaction of two or more 
controllers. Although the CAN controllers would typically belong to different participants and reside in different
processes, their interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- *canReceiver*, *canSender* are of type |ICanController|.
- All CAN controllers use the same CAN network.

Simple CAN Sender / Receiver Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one CAN controller to another CAN controller connected on the same 
CAN network.

.. literalinclude::
   examples/can/CAN_Sender_Receiver.cpp
   :language: cpp
