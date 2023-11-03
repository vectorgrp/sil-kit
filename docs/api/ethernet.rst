.. _chap:ethernet-service-api:

====================
Ethernet Service API
====================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<SilKit::IParticipant>`
.. |CreateEthernetController| replace:: :cpp:func:`CreateEthernetController<SilKit::IParticipant::CreateEthernetController()>`
.. |IEthernetController| replace:: :cpp:class:`IEthernetController<SilKit::Services::Ethernet::IEthernetController>`
.. |Activate| replace:: :cpp:func:`Activate()<SilKit::Services::Ethernet::IEthernetController::Activate>`
.. |SendFrame| replace:: :cpp:func:`SendFrame()<SilKit::Services::Ethernet::IEthernetController::SendFrame>`

.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<SilKit::Services::Ethernet::IEthernetController::AddFrameTransmitHandler>`
.. |AddStateChangeHandler| replace:: :cpp:func:`AddStateChangeHandler()<SilKit::Services::Ethernet::IEthernetController::AddStateChangeHandler>`
.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<SilKit::Services::Ethernet::IEthernetController::AddFrameHandler>`

.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<SilKit::Services::Ethernet::IEthernetController::RemoveFrameTransmitHandler>`
.. |RemoveStateChangeHandler| replace:: :cpp:func:`RemoveStateChangeHandler()<SilKit::Services::Ethernet::IEthernetController::RemoveStateChangeHandler>`
.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<SilKit::Services::Ethernet::IEthernetController::RemoveFrameHandler>`

.. |EthernetFrame| replace:: :cpp:class:`EthernetFrame<SilKit::Services::Ethernet::EthernetFrame>`
.. |EthernetFrameEvent| replace:: :cpp:class:`EthernetFrameEvent<SilKit::Services::Ethernet::EthernetFrameEvent>`
.. |EthernetFrameTransmitEvent| replace:: :cpp:class:`EthernetFrameTransmitEvent<SilKit::Services::Ethernet::EthernetFrameTransmitEvent>`
.. |EthernetTransmitStatus| replace:: :cpp:enum:`EthernetTransmitStatus<SilKit::Services::Ethernet::EthernetTransmitStatus>`

.. |Transmitted| replace:: :cpp:enumerator:`EthernetTransmitStatus::Transmitted<SilKit::Services::Ethernet::Transmitted>`
.. |ControllerInactive| replace:: :cpp:enumerator:`EthernetTransmitStatus::ControllerInactive<SilKit::Services::Ethernet::ControllerInactive>`
.. |LinkDown| replace:: :cpp:enumerator:`EthernetTransmitStatus::LinkDown<SilKit::Services::Ethernet::LinkDown>`
.. |Dropped| replace:: :cpp:enumerator:`EthernetTransmitStatus::Dropped<SilKit::Services::Ethernet::Dropped>`
.. |InvalidFrameFormat| replace:: :cpp:enumerator:`EthernetTransmitStatus::InvalidFrameFormat<SilKit::Services::Ethernet::InvalidFrameFormat>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<SilKit::Services::HandlerId>`

.. contents::
   :local:
   :depth: 3

.. highlight:: cpp

Using the Ethernet Controller
-----------------------------

The Ethernet Service API provides an Ethernet bus abstraction through the |IEthernetController| interface.
An Ethernet controller is created by calling |CreateEthernetController| given a controller name and network 
name::

  auto* ethernetController = participant->CreateEthernetController("Eth1", "Eth");

Ethernet controllers will only communicate within the same network.

Initialization
~~~~~~~~~~~~~~

The Ethernet controller first has to call |Activate| before being able to
send frames. Note that |Activate| can be called in the ``CommunicationReadyHandler`` of a ``LifecycleService``.


Sending Ethernet Frames
~~~~~~~~~~~~~~~~~~~~~~~

An |EthernetFrame| is sent with |SendFrame| and received as an |EthernetFrameEvent|. The |EthernetFrame| must be setup
according to layer 2 of IEEE 802.3, with

- a source and destination MAC address (6 octets each), 
- an optional 802.1Q tag (4 octets), 
- the `EtherType` or length (Ethernet II or IEEE 802.3, 2 octets), and
- the payload to be transmitted (46 octets or more).

.. admonition:: Note

  The frame check sequence (32-bit CRC, 4 octets) is omitted. Thus, the minimum length of a frame is 60 octets.

.. admonition:: Note

  If the frame is shorter than the minimum length of 60 octets, the frame will be padded with zeros to the minimum length.

A valid frame can be setup and sent as follows::

  // Prepare an Ethernet frame
  const std::array<uint8_t, 6> destinationAddress{ 0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc2 };
  const std::array<uint8_t, 6> sourceAddress{ 0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc1 };
  const uint16_t etherType{ 0x0800 };

  const std::string message("Ensure that the payload is long enough to constitute"
                      " a valid Ethernet frame ----------------------------");
  const std::vector<uint8_t> payload{ message.begin(), message.end() };

  EthernetFrame frame{};
  std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(frame.raw));
  std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(frame.raw));
  auto etherTypeBytes = reinterpret_cast<const uint8_t*>(&etherType);
  frame.raw.push_back(etherTypeBytes[1]);  // We assume our platform to be little-endian
  frame.raw.push_back(etherTypeBytes[0]);
  std::copy(payload.begin(), payload.end(), std::back_inserter(frame.raw));

  ethernetController->SendFrame(frame);

Transmission Acknowledgement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified of the success or failure of the transmission, a ``FrameTransmitHandler`` can be registered using
|AddFrameTransmitHandler|::
  
  auto frameTransmitHandler = [](IEthernetController*, const EthernetFrameTransmitEvent& frameTransmitEvent) 
  {
    // Handle frameTransmitEvent
  };
  ethernetController->AddFrameTransmitHandler(frameTransmitHandler);

An optional second parameter of |AddFrameTransmitHandler| allows to specify the status (|Transmitted|, ...) of the
|EthernetFrameTransmitEvent| to be received. By default, each status is enabled.

.. admonition:: Note

  In a simple simulation, the |EthernetTransmitStatus| of the 
  |EthernetFrameTransmitEvent| will always be |Transmitted|.

Receiving Ethernet Frame Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An |EthernetFrame| is received as an |EthernetFrameEvent| consisting of a ``transmitId`` used to identify
the acknowledgement of the frame, a timestamp and the actual |EthernetFrame|.

To receive Ethernet frames, a frame handler must be registered using |AddFrameHandler|. The handler is called whenever 
an Ethernet frame is received::

  auto frameHandler = [](IEthernetController*, const EthernetFrameEvent& frameEvent) 
  {
    // Handle frameEvent
  };
  ethernetController->AddFrameHandler(frameHandler);

An optional second parameter of |AddFrameHandler| allows to specify the direction (Tx, Rx, Tx/Rx) of the Ethernet frames to be
received. By default, only frames of Rx direction are handled.

Managing the Event Handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameHandler|
- |RemoveFrameTransmitHandler|  
- |RemoveStateChangeHandler|

Switches
________

Switches can be used in a detailed simulation.
Refer to the documentation of the network simulator for further information.

Receiving State Change Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive state changes of an Ethernet 
controller, a ``StateChangeHandler`` must be registered using |AddStateChangeHandler|::

  auto stateChangedHandler = [](IEthernetController*, const EthernetStateChangeEvent& stateChangeEvent) 
  {
    // Handle stateChangeEvent;
  };
  ethernetController->AddStateChangeHandler(stateChangedHandler);

Acknowledgements
~~~~~~~~~~~~~~~~

When sending frames, the |EthernetTransmitStatus| of the |EthernetFrameTransmitEvent| received in the
``FrameTransmitHandler`` will be one of the following values:

- |Transmitted|: Transmission was successful.
- |ControllerInactive|: The sending Ethernet controller tried to send a frame before |Activate| was called.
- |LinkDown|: |Activate| has been called but the link to another Ethernet Controller has not yet been established.
- |Dropped|: Indicates a transmit queue overflow.
- |InvalidFrameFormat|: The Ethernet frame is invalid, e.g., too small or too large.

API and Data Type Reference
---------------------------

Ethernet Controller API
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: SilKit::Services::Ethernet::IEthernetController
   :members:

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: SilKit::Services::Ethernet::EthernetFrame
   :members:
.. doxygenstruct:: SilKit::Services::Ethernet::EthernetFrameEvent
   :members:
.. doxygenstruct:: SilKit::Services::Ethernet::EthernetFrameTransmitEvent
   :members:
.. doxygenstruct:: SilKit::Services::Ethernet::EthernetStateChangeEvent
   :members:
.. doxygenstruct:: SilKit::Services::Ethernet::EthernetBitrateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: SilKit::Services::Ethernet::EthernetBitrate
.. doxygenenum:: SilKit::Services::Ethernet::EthernetTransmitStatus
.. doxygenenum:: SilKit::Services::Ethernet::EthernetState

Usage Examples
--------------

This section contains complete examples that show the usage and the interaction of two Ethernet controllers. Although 
the Ethernet controllers would typically belong to different participants and reside in different processes, their 
interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- Variables ``ethernetReceiver``, ``ethernetSender`` are of type |IEthernetController|.
- All Ethernet controllers are connected to the same switch.

Simple Ethernet Sender / Receiver Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one Ethernet controller to another.

.. literalinclude::
   examples/eth/ETH_Sender_Receiver.cpp
   :language: cpp

State Transition Example
~~~~~~~~~~~~~~~~~~~~~~~~

This example shows the possible state transitions for an Ethernet controller.

.. literalinclude::
   examples/eth/ETH_State_Transitions.cpp
   :language: cpp


Erroneous Transmissions (Detailed Simulation only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows different possible erroneous Ethernet transmissions.

.. literalinclude::
   examples/eth/ETH_Erroneous_Transmissions.cpp
   :language: cpp
