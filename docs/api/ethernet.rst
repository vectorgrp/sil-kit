====================
Ethernet Service API
====================


.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the Ethernet Controller
------------------------------------

Initialization
~~~~~~~~~~~~~~~~~~~~

For a detailed simulation with the :ref:`VIBE Network Simulator<chap:VIBE-NetSim>`, the link must first be established
by calling :cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>` before sending messages::

    ethernetController->Activate();

Note that :cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>`
can be called in the InitHandler of a ParticipantController. 
In a simple functional simulation without :ref:`VIBE Network Simulator<chap:VIBE-NetSim>` this function performs no 
operation.

.. admonition:: Note

  If the VIBE NetworkSimulator is used, an Ethernet Controller should be connected to a switch
  for a valid simulation (for configuration details, refer to the sections :ref:`Switches<sec:cfg-switches>`
  and :ref:`Network Simulators<sec:cfg-network-simulators>`).


Sending Ethernet Frames
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An Ethernet Controller can send :cpp:class:`EthFrame<ib::sim::eth::EthFrame>`s and receive 
:cpp:class:`EthMessage<ib::sim::eth::EthMessage>`s. In addition to the :cpp:class:`EthFrame<ib::sim::eth::EthFrame>`, 
the :cpp:class:`EthMessage<ib::sim::eth::EthMessage>` consists of a transmitId, used to identify
the acknowledge of the message and an optional timestamp.
To send an Ethernet Frame, the :cpp:class:`EthFrame<ib::sim::eth::EthFrame>`
must be setup with a source and destination MAC address and the payload to be transmitted::

  // Prepare an Ethernet frame
  std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
  std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

  std::string message("Ensure that the payload is long enough to constitute"
                      " a valid ethernet frame ----------------------------");
  std::vector<uint8_t> payload{message.begin(), message.end()};

  EthFrame ethFrame;
  ethFrame.SetSourceMac(sourceAddress);
  ethFrame.SetDestinationMac(destinationAddress);
  ethFrame.SetPayload(payload);

  ethernetController->SendFrame(ethFrame);

To be notified for the success or failure of the transmission, a MessageAckHandler should
be registered::
  
  // Register MessageAckHandler to receive Ethernet acknowledges from other Ethernet controller.
  auto messageAckHandler =
      [](IEthController*, const EthTransmitAcknowledge& ack) {};
  ethernetController->RegisterMessageAckHandler(messageAckHandler);

The :cpp:class:`EthTransmitAcknowledge<ib::sim::eth::EthTransmitAcknowledge>` received in the MessageAckHandler
will always contain an EthTransmitStatus with the value 
:cpp:enumerator:`Transmitted<ib::sim::eth::Transmitted>` in a simple simulation without VIBE NetworkSimulator,
which indicates a successful transmission.

If the VIBE NetworkSimulator is used, other status values are possible.
:cpp:enumerator:`ControllerInactive<ib::sim::eth::ControllerInactive>` is returned
as long as an Ethernet Controller tries to send messages before
:cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>` is called. If
:cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>` has been called,
the EthTransmitStatus for sent messages will be
:cpp:enumerator:`LinkDown<ib::sim::eth::LinkDown>` as long as the Ethernet link to another
Ethernet Controller has not yet been established. Furthermore, it is possible that the transmit queue
overflows causing the handler to be called with :cpp:enumerator:`Dropped<ib::sim::eth::Dropped>`.
Finally, :cpp:enumerator:`InvalidFrameFormat<ib::sim::eth::InvalidFrameFormat>` is returned, e.g.
if the Ethernet frame is too small or too large.


Receiving Ethernet Messages or EthState changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To receive Ethernet frames from other Ethernet controller, a ReceiveMessageHandler must be
registered, which is called by the Ethernet controller whenever an Ethernet message is received::

  // Register ReceiveMessageHandler to receive Ethernet messages from other CAN controller.
  auto receiveMessageHandler =
      [](IEthController*, const EthMessage&) {};
  ethernetController->RegisterReceiveMessageHandler(receiveMessageHandler);

To receive EthState changes of an Ethernet controller, a StateChangedHandler
must be registered, which is called whenever the status changes::

  // Register StateChangedHandler to receive EthState changes from the Ethernet controller.
  auto stateChangedHandler =
      [](IEthController*, const EthState&) {};
  ethernetController->RegisterStateChangedHandler(stateChangedHandler);

.. admonition:: Note

  State changes are only supported when using the VIBE NetworkSimulator.


.. _sec:api-ethernet-tracing:

Message Tracing
~~~~~~~~~~~~~~~

.. admonition:: Note

  Currently the Message Tracing functionality is not available, but it will be reintegrated in the future.

The Ethernet Controller is able to trace all received Ethernet messages in PCAP format, either
in a dedicated file or into a named pipe.
MDF4 tracing is supported by the :ref:`VIBE MDF4Tracing<mdf4tracing>`.
By default, message tracing is disabled, but it can be enabled in the settings
of an Ethernet Controller (see: :ref:`Ethernet Controller Configuration<sec:cfg-participant-ethernet>`).
Refer to the :ref:`sec:cfg-participant-tracing` configuration section for usage instructions.

PCAP File
__________

To trace all received Ethernet messages in a PCAP file, you have to specify a trace sink
of type 'PcapFile' in the configuration of the Ethernet Controller and add an appropriate
trace sink to the configuration:

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
  

After you successfully ran and stopped the simulation, you will find the file
"Ethernet.pcap" in the simulation's working directory.
It can be loaded into a tool like
`Wireshark <https://www.wireshark.org/>`_ where you can examine the Ethernet trace.

PCAP Named Pipe
_________________

Using a named pipe allows attaching another program to trace messages of a
IB ethernet controller. 
The trace sink type has to be specified as "PcapPipe" in the configuration:

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
    

The VIB process responsible for the Ethernet Controller "ETH0" will open the
specified named pipe "EthernetPipe" during start up of the Participant.
When the IntegrationBus writes the first message to the pipe, the VIB process will be blocked
until another process connects to the named pipe and reads the traced ethernet
messages from the pipe.

The reading process could be a tool like `Wireshark <https://www.wireshark.org/>`_,
which allows visualizing live trace messages.
Under Windows, named pipes reside in a special filesystem namespace prefixed with "\\.\pipe\".
The following will attach *wireshark* to the named pipe created by your VIB simulation:

.. code-block:: powershell

  # Start wireshark and start reading from the named pipe
  wireshark -ni \\.\pipe\EthernetPipe


API and Data Type Reference
--------------------------------------------------
Ethernet Controller API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenclass:: ib::sim::eth::IEthController
   :members:

Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenstruct:: ib::sim::eth::EthMessage
   :members:
.. doxygenclass:: ib::sim::eth::EthFrame
   :members:
.. doxygenstruct:: ib::sim::eth::EthTagControlInformation
   :members:
.. doxygenstruct:: ib::sim::eth::EthTransmitAcknowledge
   :members:
.. doxygenstruct:: ib::sim::eth::EthStatus
   :members:

Enumerations and Typedefs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygentypedef:: ib::sim::eth::EthTxId
.. doxygentypedef:: ib::sim::eth::EthMac
.. doxygentypedef:: ib::sim::eth::EthVid
.. doxygenenum:: ib::sim::eth::EthTransmitStatus
.. doxygenenum:: ib::sim::eth::EthState


Usage Examples
----------------------------------------------------

This section contains complete examples that show the usage and the interaction
of two Ethernet controllers. Although the Ethernet controllers would
typically belong to different participants and reside in different processes,
their interaction is shown sequentially to demonstrate cause and effect.

Assumptions:

- *ethernetReceiver*, *ethernetSender* are of type
  :cpp:class:`IEthController*<ib::sim::eth::IEthController>`.
- All Ethernet controllers are connected to the same switch.

Simple Ethernet Sender / Receiver Example (without VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a successful data transfer from one Ethernet controller
to another. For simplicity this example is considered to be without the VIBE NetworkSimulator
so that an :cpp:class:`EthTransmitAcknowledge<ib::sim::eth::EthTransmitAcknowledge>`
will always return :cpp:enumerator:`EthTransmitStatus::Transmitted<ib::sim::eth::Transmitted>`.

.. literalinclude::
   examples/eth/ETH_Sender_Receiver.cpp
   :language: cpp


State Transition Example (only with VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows the possible state transitions for an Ethernet controller.

.. literalinclude::
   examples/eth/ETH_State_Transitions.cpp
   :language: cpp


Erroneous Transmissions (only with VIBE NetworkSimulator)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows different possible erroneous Ethernet transmissions 
when using the VIBE NetworkSimulator.

.. literalinclude::
   examples/eth/ETH_Erroneous_Transmissions.cpp
   :language: cpp
