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

.. |EthernetFrame| replace:: :cpp:class:`EthernetFrame<ib::sim::eth::EthernetFrame>`
.. |EthernetFrameEvent| replace:: :cpp:class:`EthernetFrameEvent<ib::sim::eth::EthernetFrameEvent>`
.. |EthernetFrameTransmitEvent| replace:: :cpp:class:`EthernetFrameTransmitEvent<ib::sim::eth::EthernetFrameTransmitEvent>`
.. |EthernetTransmitStatus| replace:: :cpp:class:`EthernetTransmitStatus<ib::sim::eth::EthernetTransmitStatus>`

.. |Transmitted| replace:: :cpp:enumerator:`EthernetTransmitStatus::Transmitted<ib::sim::eth::Transmitted>`
.. |ControllerInactive| replace:: :cpp:enumerator:`EthernetTransmitStatus::ControllerInactive<ib::sim::eth::ControllerInactive>`
.. |LinkDown| replace:: :cpp:enumerator:`EthernetTransmitStatus::LinkDown<ib::sim::eth::LinkDown>`
.. |Dropped| replace:: :cpp:enumerator:`EthernetTransmitStatus::Dropped<ib::sim::eth::Dropped>`
.. |InvalidFrameFormat| replace:: :cpp:enumerator:`EthernetTransmitStatus::InvalidFrameFormat<ib::sim::eth::InvalidFrameFormat>`

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

Sending Ethernet Frames
~~~~~~~~~~~~~~~~~~~~~~~

An |EthernetFrame| is sent with |SendFrame| and received as an |EthernetFrameEvent|. The |EthernetFrame| must be setup
with a source and destination MAC address and the payload to be transmitted::

  // Prepare an Ethernet frame
  std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
  std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

  std::string message("Ensure that the payload is long enough to constitute"
                      " a valid Ethernet frame ----------------------------");
  std::vector<uint8_t> payload{message.begin(), message.end()};

  EthernetFrame ethFrame;
  ethFrame.SetSourceMac(sourceAddress);
  ethFrame.SetDestinationMac(destinationAddress);
  ethFrame.SetPayload(payload);

  ethernetController->SendFrame(ethFrame);

Transmission acknowledgement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To be notified of the success or failure of the transmission, a ``FrameTransmitHandler`` can be registered using
|AddFrameTransmitHandler|::
  
  auto frameTransmitHandler = [](IEthernetController*, const EthernetFrameTransmitEvent& frameTransmitEvent) 
  {
    // Handle frameTransmitEvent
  };
  ethernetController->AddFrameTransmitHandler(frameTransmitHandler);

.. admonition:: Note

  In a simple simulation without the VIBE NetworkSimulator, the |EthernetTransmitStatus| of the 
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

Usage with the VIBE NetworkSimulator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Initialization
______________

If used within a simulated Ethernet network, the Ethernet controller first has to call |Activate| before being able to
send frames. Note that |Activate| can be called in the InitHandler of a ParticipantController.

Switches
________

An Ethernet controller should be connected to a switch for a valid simulation. For further details, refer to 
:ref:`simulating Ethernet switches<vibes/networksimulator:Ethernet>` and the 
:ref:`Network Simulator configuration<vibes/networksimulator:Configuration>`.

Receive state change events
___________________________

State changes are only supported when using the VIBE NetworkSimulator. To receive state changes of an Ethernet 
controller, a ``StateChangeHandler`` must be registered using |AddStateChangeHandler|::

  auto stateChangedHandler = [](IEthernetController*, const EthernetStateChangeEvent& stateChangeEvent) 
  {
    // Handle stateChangeEvent;
  };
  ethernetController->AddStateChangeHandler(stateChangedHandler);

Acknowledgements
________________

When sending frames, the |EthernetTransmitStatus| of the |EthernetFrameTransmitEvent| received in the
``FrameTransmitHandler`` will be one of the following values:

- |Transmitted|: Successful transmission.
- |ControllerInactive|: The sending Ethernet controller tried to send a frame before |Activate| was called.
- |LinkDown|: |Activate| has been called but the link to another Ethernet Controller has not yet been established.
- |Dropped|: Indicates a transmit queue overflow.
- |InvalidFrameFormat|: The Ethernet frame is invalid, e.g. too small or too large.

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
          "MacAddress": "00:08:15:ab:cd:f0",
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
          "MacAddress": "00:08:15:ab:cd:f0",
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

.. doxygenstruct:: ib::sim::eth::EthernetFrameEvent
   :members:
.. doxygenclass:: ib::sim::eth::EthernetFrame
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetTagControlInformation
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetFrameTransmitEvent
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetStateChangeEvent
   :members:
.. doxygenstruct:: ib::sim::eth::EthernetBitrateChangeEvent
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: ib::sim::eth::EthernetTxId
.. doxygentypedef:: ib::sim::eth::EthernetMac
.. doxygentypedef:: ib::sim::eth::EthernetVid
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

Simple Ethernet Sender / Receiver Example (without VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one Ethernet controller to another. For simplicity this example is 
considered to be without the VIBE NetworkSimulator so that an |EthernetFrameTransmitEvent| will always return 
|Transmitted|

.. literalinclude::
   examples/eth/ETH_Sender_Receiver.cpp
   :language: cpp


State Transition Example (only with VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows the possible state transitions for an Ethernet controller.

.. literalinclude::
   examples/eth/ETH_State_Transitions.cpp
   :language: cpp


Erroneous Transmissions (only with VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows different possible erroneous Ethernet transmissions when using the VIBE NetworkSimulator.

.. literalinclude::
   examples/eth/ETH_Erroneous_Transmissions.cpp
   :language: cpp
