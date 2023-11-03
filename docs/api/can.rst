.. _chap:can-service-api:

===================
CAN Service API
===================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateCanController| replace:: :cpp:func:`CreateCanController<SilKit::IParticipant::CreateCanController()>`
.. |ICanController| replace:: :cpp:class:`ICanController<SilKit::Services::Can::ICanController>`

.. |SendFrame| replace:: :cpp:func:`SendFrame()<SilKit::Services::Can::ICanController::SendFrame>`
.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<SilKit::Services::Can::ICanController::AddFrameTransmitHandler>`
.. |AddStateChangeHandler| replace:: :cpp:func:`AddStateChangeHandler()<SilKit::Services::Can::ICanController::AddStateChangeHandler>`
.. |AddErrorStateChangeHandler| replace:: :cpp:func:`AddErrorStateChangeHandler()<SilKit::Services::Can::ICanController::AddErrorStateChangeHandler>`
.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<SilKit::Services::Can::ICanController::AddFrameHandler>`
.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<SilKit::Services::Can::ICanController::RemoveFrameTransmitHandler>`
.. |RemoveStateChangeHandler| replace:: :cpp:func:`RemoveStateChangeHandler()<SilKit::Services::Can::ICanController::RemoveStateChangeHandler>`
.. |RemoveErrorStateChangeHandler| replace:: :cpp:func:`RemoveErrorStateChangeHandler()<SilKit::Services::Can::ICanController::RemoveErrorStateChangeHandler>`
.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<SilKit::Services::Can::ICanController::RemoveFrameHandler>`
.. |Start| replace:: :cpp:func:`Start()<SilKit::Services::Can::ICanController::Start>`
.. |Stop| replace:: :cpp:func:`Stop()<SilKit::Services::Can::ICanController::Stop>`
.. |Reset| replace:: :cpp:func:`Reset()<SilKit::Services::Can::ICanController::Reset>`
.. |SetBaudRate| replace:: :cpp:func:`ICanController::SetBaudRate()<SilKit::Services::Can::ICanController::SetBaudRate>`

.. |CanFrame| replace:: :cpp:class:`CanFrame<SilKit::Services::Can::CanFrame>`
.. |CanFrameEvent| replace:: :cpp:class:`CanFrameEvent<SilKit::Services::Can::CanFrameEvent>`
.. |CanFrameTransmitEvent| replace:: :cpp:class:`CanFrameTransmitEvent<SilKit::Services::Can::CanFrameTransmitEvent>`
.. |CanStateChangeEvent| replace:: :cpp:class:`CanStateChangeEvent<SilKit::Services::Can::CanStateChangeEvent>`
.. |CanErrorStateChangeEvent| replace:: :cpp:class:`CanErrorStateChangeEvent<SilKit::Services::Can::CanErrorStateChangeEvent>`

.. |CanControllerState| replace:: :cpp:enum:`CanControllerState<SilKit::Services::Can::CanControllerState>`
.. |CanErrorState| replace:: :cpp:enum:`CanErrorState<SilKit::Services::Can::CanErrorState>`
.. |CanFrameFlag| replace:: :cpp:class:`CanFrame::CanFrameFlag<SilKit::Services::Can::CanFrame::CanFrameFlag>`
.. |CanTransmitStatus| replace:: :cpp:enum:`CanTransmitStatus<SilKit::Services::Can::CanTransmitStatus>`

.. |Transmitted| replace:: :cpp:enumerator:`CanTransmitStatus::Transmitted<SilKit::Services::Can::Transmitted>`
.. |Canceled| replace:: :cpp:enumerator:`CanTransmitStatus::Canceled<SilKit::Services::Can::Canceled>`
.. |TransmitQueueFull| replace:: :cpp:enumerator:`CanTransmitStatus::TransmitQueueFull<SilKit::Services::Can::TransmitQueueFull>`
.. |DuplicatedTransmitId| replace:: :cpp:enumerator:`CanTransmitStatus::DuplicatedTransmitId<SilKit::Services::Can::DuplicatedTransmitId>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<SilKit::Services::HandlerId>`

.. |_| unicode:: 0xA0 
   :trim:

.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the CAN Controller
-------------------------

The CAN Service API provides a CAN bus abstraction through the |ICanController| interface.
A CAN controller is created by calling |CreateCanController| given a controller name and network 
name::

  auto* canController = participant->CreateCanController("CAN1", "CAN");

CAN controllers will only communicate within the same network.

Sending CAN Frames
~~~~~~~~~~~~~~~~~~

Data is transferred in the form of a |CanFrame| and received as a |CanFrameEvent|. To send a |CanFrame|, it must be setup 
with a CAN ID and the data to be transmitted. Furthermore, valid |CanFrameFlag| have to be set::

  // Prepare a CAN message with id 0x17
  CanFrame canFrame;
  canFrame.canId = 3;
  canFrame.flags = static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)  // FD Format Indicator
                 | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs); // Bit Rate Switch (for FD Format only)
  canFrame.dataField = {'d', 'a', 't', 'a', 0, 1, 2, 3};

  canController.SendFrame(canFrame);

Transmission Acknowledgement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified of the success or failure of the transmission, a ``FrameTransmitHandler`` can be registered using
|AddFrameTransmitHandler|::

  auto frameTransmitHandler = [](ICanController*, const CanFrameTransmitEvent& frameTransmitEvent) 
  {
    // Handle frameTransmitEvent
  };
  canController->AddFrameTransmitHandler(frameTransmitHandler);

An optional second parameter of |AddFrameTransmitHandler| allows to specify the status (|Transmitted|, ...) of the
|CanFrameTransmitEvent| to be received. By default, each status is enabled.

.. admonition:: Note

  In a simple simulation without the network simulator, the |CanTransmitStatus| of the |CanFrameTransmitEvent| will
  always be |Transmitted|. If a detailed simulation is used, it is possible that the transmit queue overflows
  causing the handler to be called with |TransmitQueueFull| signaling a transmission failure.

Receiving CAN Frame Events
~~~~~~~~~~~~~~~~~~~~~~~~~~

A |CanFrame| is received as a |CanFrameEvent| consisting of a ``transmitId`` used to identify the acknowledgement of the 
frame, a timestamp and the actual |CanFrame|. The handler is called whenever a |CanFrame| is received::

  auto frameHandler = [](ICanController*, const CanFrameEvent& frameEvent) 
  {
    // Handle frameEvent
  };
  canController->AddFrameHandler(frameHandler);

An optional second parameter of |AddFrameHandler| allows to specify the direction (TX, RX, TX/RX) of the CAN frames to be
received. By default, only frames of RX direction are handled.

Receiving State Change Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive changes of the |CanControllerState|,
a ``StateChangeHandler`` must be registered using |AddStateChangeHandler|::

  auto stateChangedHandler = [](ICanController*, const CanStateChangeEvent& stateChangeEvent) 
  {
    // Handle stateChangeEvent;
  };
  canController->AddStateChangeHandler(stateChangedHandler);

Similarly, changes in the |CanErrorState| can be tracked with |AddErrorStateChangeHandler|.

Initialization
~~~~~~~~~~~~~~

A CAN controller's baud rate must first be configured by passing a value to |SetBaudRate|.
Then, the controller must be started explicitly by calling |Start|. Now the controller can be used.
Additional control commands are |Stop| and |Reset|.

The following example configures a CAN controller with a baud rate of 10'000 baud for regular CAN messages and a baud 
rate of 1'000'000 baud for CAN |_| FD messages. Then, the controller is started::

    canController->SetBaudRate(10000, 1000000);
    canController->Start();

.. admonition:: Note

   Both |SetBaudRate| and |Start| should not be called earlier than in the lifecycle service's
   :cpp:func:`communication ready handler<SilKit::Core::synd::ILifecycleService::SetCommunicationReadyHandler()>`. Otherwise, it is not guaranteed 
   that all participants are already connected, which can cause the call to have no effect.

Managing the Event Handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId|. This ID can be used to remove the handler via:

- |RemoveFrameTransmitHandler|
- |RemoveStateChangeHandler|
- |RemoveErrorStateChangeHandler|
- |RemoveFrameHandler|

API and Data Type Reference
---------------------------
CAN Controller API
~~~~~~~~~~~~~~~~~~
.. doxygenclass:: SilKit::Services::Can::ICanController
   :members:

Data Structures
~~~~~~~~~~~~~~~
.. doxygenstruct:: SilKit::Services::Can::CanFrame
   :members:
.. doxygenstruct:: SilKit::Services::Can::CanFrameEvent
   :members:
.. doxygenstruct:: SilKit::Services::Can::CanFrameTransmitEvent
   :members:
.. doxygenstruct:: SilKit::Services::Can::CanStateChangeEvent
   :members:
.. doxygenstruct:: SilKit::Services::Can::CanErrorStateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: SilKit::Services::Can::CanControllerState
.. doxygenenum:: SilKit::Services::Can::CanErrorState
.. doxygenenum:: SilKit::Services::Can::CanFrameFlag
.. doxygenenum:: SilKit::Services::Can::CanTransmitStatus

Usage Examples
--------------

This section contains complete examples that show the usage of the CAN controller and the interaction of two or more 
controllers. Although the CAN controllers would typically belong to different participants and reside in different
processes, their interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- Variables ``canReceiver`` and ``canSender`` are of type |ICanController|.
- All CAN controllers use the same CAN network.

Simple CAN Sender / Receiver Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one CAN controller to another CAN controller connected on the same 
CAN network.

.. literalinclude::
   examples/can/CAN_Sender_Receiver.cpp
   :language: cpp
