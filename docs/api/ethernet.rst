====================
Ethernet Service API
====================

.. Macros for docs use
.. |IParticipant| replace:: :cpp:class:`IParticipant<ib::mw::IParticipant>`
.. |CreateEthernetController| replace:: :cpp:func:`CreateEthernetController<ib::mw::IParticipant::CreateEthernetController()>`
.. |IEthernetController| replace:: :cpp:class:`IEthernetController<ib::sim::eth::IEthernetController>`
.. |Activate| replace:: :cpp:func:`Activate()<ib::sim::eth::IEthernetController::Activate>`
.. |SendFrame| replace:: :cpp:func:`SendFrame()<ib::sim::eth::IEthernetController::SendFrame>`

.. |AddFrameTransmitHandler| replace:: :cpp:func:`AddFrameTransmitHandler()<ib::sim::eth::IEthernetController::AddFrameTransmitHandler>`
.. |AddStateChangeHandler| replace:: :cpp:func:`AddStateChangeHandler()<ib::sim::eth::IEthernetController::AddStateChangeHandler>`
.. |AddFrameHandler| replace:: :cpp:func:`AddFrameHandler()<ib::sim::eth::IEthernetController::AddFrameHandler>`

.. |RemoveFrameTransmitHandler| replace:: :cpp:func:`RemoveFrameTransmitHandler()<ib::sim::eth::IEthernetController::RemoveFrameTransmitHandler>`
.. |RemoveStateChangeHandler| replace:: :cpp:func:`RemoveStateChangeHandler()<ib::sim::eth::IEthernetController::RemoveStateChangeHandler>`
.. |RemoveFrameHandler| replace:: :cpp:func:`RemoveFrameHandler()<ib::sim::eth::IEthernetController::RemoveFrameHandler>`

.. |EthernetFrame| replace:: :cpp:class:`EthernetFrame<ib::sim::eth::EthernetFrame>`
.. |EthernetFrameEvent| replace:: :cpp:class:`EthernetFrameEvent<ib::sim::eth::EthernetFrameEvent>`
.. |EthernetFrameTransmitEvent| replace:: :cpp:class:`EthernetFrameTransmitEvent<ib::sim::eth::EthernetFrameTransmitEvent>`
.. |EthernetTransmitStatus| replace:: :cpp:enum:`EthernetTransmitStatus<ib::sim::eth::EthernetTransmitStatus>`

.. |Transmitted| replace:: :cpp:enumerator:`EthernetTransmitStatus::Transmitted<ib::sim::eth::Transmitted>`
.. |ControllerInactive| replace:: :cpp:enumerator:`EthernetTransmitStatus::ControllerInactive<ib::sim::eth::ControllerInactive>`
.. |LinkDown| replace:: :cpp:enumerator:`EthernetTransmitStatus::LinkDown<ib::sim::eth::LinkDown>`
.. |Dropped| replace:: :cpp:enumerator:`EthernetTransmitStatus::Dropped<ib::sim::eth::Dropped>`
.. |InvalidFrameFormat| replace:: :cpp:enumerator:`EthernetTransmitStatus::InvalidFrameFormat<ib::sim::eth::InvalidFrameFormat>`

.. |HandlerId| replace:: :cpp:class:`HandlerId<ib::sim::HandlerId>`

.. contents::
   :local:
   :depth: 3

.. highlight:: cpp

Using the Ethernet Controller
-----------------------------

The Ethernet Service API provides an Ethernet bus abstraction through the |IEthernetController| interface.
An Ethernet controller is created by calling |CreateEthernetController| given a controller name and (optional) network 
name::

  auto* ethernetController = participant->CreateEthernetController("Eth1", "Eth");

Ethernet controllers will only communicate within the same network. If no network name is provided, the controller name
will be used as the network name.

Initialization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Ethernet controller first has to call |Activate| before being able to
send frames. Note that |Activate| can be called in the CommunicationReadyHandler of a LifecycleService.


Sending Ethernet Frames
~~~~~~~~~~~~~~~~~~~~~~~

An |EthernetFrame| is sent with |SendFrame| and received as an |EthernetFrameEvent|. The |EthernetFrame| must be setup
according to layer 2 of IEEE 802.3, with

- a source and destination MAC address (6 octets each), 
- an optional 802.1Q tag (4 octets), 
- the Ethertype or length (Ethernet II or IEEE 802.3, 2 octets), and
- the payload to be transmitted (46 octets or more).

.. admonition:: Note

  The frame check sequence (32-bit CRC, 4 octets) is omitted. Thus, the minimum length of a frame is 60 octets.

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

.. admonition:: Note

  In a simple simulation, the |EthernetTransmitStatus| of the 
  |EthernetFrameTransmitEvent| will always be |Transmitted|. 

Receiving Ethernet FrameEvents
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An |EthernetFrame| is received as an |EthernetFrameEvent| consisting of a transmitId used to identify
the acknowledge of the frame, a timestamp and the actual |EthernetFrame|.

To receive Ethernet frames, a FrameHandler must be registered using |AddFrameHandler|. The handler is called whenever 
an Ethernet frame is received::

  auto frameHandler = [](IEthernetController*, const EthernetFrameEvent& frameEvent) 
  {
    // Handle frameEvent
  };
  ethernetController->AddFrameHandler(frameHandler);

Managing the event handlers
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Adding a handler will return a |HandlerId| which can be used to remove the handler via:

- |RemoveFrameHandler|
- |RemoveFrameTransmitHandler|  
- |RemoveStateChangeHandler|

Switches
________

Switches can be used in a detailed simulation. 
Refer to the documentation of the network simulator for further information.

Receive State Change Events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive state changes of an Ethernet 
controller, a ``StateChangeHandler`` must be registered using |AddStateChangeHandler|::

  auto stateChangedHandler = [](IEthernetController*, const EthernetStateChangeEvent& stateChangeEvent) 
  {
    // Handle stateChangeEvent;
  };
  ethernetController->AddStateChangeHandler(stateChangedHandler);

Acknowledgements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When sending frames, the |EthernetTransmitStatus| of the |EthernetFrameTransmitEvent| received in the
``FrameTransmitHandler`` will be one of the following values:

- |Transmitted|: Successful transmission.
- |ControllerInactive|: The sending Ethernet controller tried to send a frame before |Activate| was called.
- |LinkDown|: |Activate| has been called but the link to another Ethernet Controller has not yet been established.
- |Dropped|: Indicates a transmit queue overflow.
- |InvalidFrameFormat|: The Ethernet frame is invalid, e.g. too small or too large.

.. _sec:api-ethernet-tracing:

Message Tracing
~~~~~~~~~~~~~~~

.. admonition:: Note
  
  Currently the Message Tracing functionality is not available, but it will be reintegrated in the future.

The Ethernet Controller is able to trace all received Ethernet frames in PCAP format, either in a dedicated file or 
into a named pipe. MDF4 tracing is supported by the :ref:`VIBE MDF4Tracing<mdf4tracing>`. By default, message tracing
is disabled, but it can be enabled in the settings of an Ethernet Controller 
(see: :ref:`Ethernet Controller Configuration<sec:cfg-participant-ethernet>`). Refer to the 
:ref:`sec:cfg-participant-tracing` configuration section for usage instructions.

PCAP File
_________

To trace all received Ethernet frames in a PCAP file, you have to specify a trace sink of type 'PcapFile' in the 
configuration of the Ethernet Controller and add an appropriate trace sink to the configuration:

.. code-block:: javascript

  "EthernetControllers": [
      {
          "Name": "ETH0",
          "UseTraceSinks": ["SinkName"]
      }
  ],
  "TraceSinks" : [
      {
          "Name" : "SinkName",
          "Type" : "PcapFile",
          "OutputPath": "Ethernet.pcap"
      }
  ]
  

After you successfully ran and stopped the simulation, you will find the file "Ethernet.pcap" in the simulation's 
working directory. It can be loaded into a tool like `Wireshark <https://www.wireshark.org/>`_ where you can examine 
the Ethernet trace.

PCAP Named Pipe
_______________

Using a named pipe allows attaching another program to trace frames of an Ethernet controller. The trace sink type
has to be specified as "PcapPipe" in the configuration:

.. code-block:: javascript

  "EthernetControllers": [
      {
          "Name": "ETH0",
          "UseTraceSinks": ["SinkName"]
      }
  ],
  "TraceSinks" : [
      {
          "Name" : "SinkName",
          "Type" : "PcapPipe",
          "OutputPath": "EthernetPipe"
      }
  ]
    

The VIB process responsible for the Ethernet Controller "ETH0" will open the specified named pipe "EthernetPipe" during
start up of the Participant. When the IntegrationBus writes the first message to the pipe, the VIB process will be 
blocked until another process connects to the named pipe and reads the traced messages from the pipe.

The reading process could be a tool like `Wireshark <https://www.wireshark.org/>`_, which allows visualizing live trace
messages. Under Windows, named pipes reside in a special filesystem namespace prefixed with "\\.\pipe\". The following 
snippet will attach *wireshark* to the named pipe created by your VIB simulation:

.. code-block:: powershell

  # Start wireshark and start reading from the named pipe
  wireshark -ni \\.\pipe\EthernetPipe


API and Data Type Reference
---------------------------

Ethernet Controller API
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: ib::sim::eth::IEthernetController
   :members:

Data Structures
~~~~~~~~~~~~~~~

.. doxygenstruct:: ib::sim::eth::EthernetFrame
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetFrameEvent
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetFrameTransmitEvent
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetStateChangeEvent
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetBitrateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: ib::sim::eth::EthernetMac
.. doxygentypedef:: ib::sim::eth::EthernetTxId
.. doxygentypedef:: ib::sim::eth::EthernetBitrate
.. doxygenenum:: ib::sim::eth::EthernetTransmitStatus
.. doxygenenum:: ib::sim::eth::EthernetState

Usage Examples
--------------

This section contains complete examples that show the usage and the interaction of two Ethernet controllers. Although 
the Ethernet controllers would typically belong to different participants and reside in different processes, their 
interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- *ethernetReceiver*, *ethernetSender* are of type |IEthernetController|.
- All Ethernet controllers are connected to the same switch.

Simple Ethernet Sender / Receiver Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one Ethernet controller to another. 

.. literalinclude::
   examples/eth/ETH_Sender_Receiver.cpp
   :language: cpp


State Transition Example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows the possible state transitions for an Ethernet controller.

.. literalinclude::
   examples/eth/ETH_State_Transitions.cpp
   :language: cpp


Erroneous Transmissions (detailed simulation only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows different possible erroneous Ethernet transmissions.

.. literalinclude::
   examples/eth/ETH_Erroneous_Transmissions.cpp
   :language: cpp
