=====================
Ethernet API
=====================


.. contents::
   :local:
   :depth: 3


.. highlight:: cpp

Using the Ethernet Controller
------------------------------------

Initialization
~~~~~~~~~~~~~~~~~~~~

Before the Ethernet Controller can send messages, the link must first be established
by calling :cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>`::

    ethernetController->Activate();

Note that :cpp:func:`IEthController::Activate()<ib::sim::eth::IEthController::Activate>`
can be called in the InitHandler of a ParticipantController.

.. admonition:: Note

  If the VIBE NetworkSimulator is used, an Ethernet Controller should be connected to a switch
  for a valid simulation (for configuration details, refer to the sections :ref:`Switches<sec:cfg-switches>`
  and :ref:`Network Simulators<sec:cfg-network-simulators>`).


Sending Ethernet Messages
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Data is transfered in the form of an :cpp:class:`EthMessage<ib::sim::eth::EthMessage>`.
The :cpp:class:`EthMessage<ib::sim::eth::EthMessage>` consists of a transmitId, used to identify
the acknowledge of the message, an optional timestamp and an :cpp:class:`EthFrame<ib::sim::eth::EthFrame>`.
To send an :cpp:class:`EthMessage<ib::sim::eth::EthMessage>`, the :cpp:class:`EthFrame<ib::sim::eth::EthFrame>`
must be setup with a source and destination MAC address and the payload to be transmitted::

  // Prepare an Ethernet message
  std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
  std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

  std::string message("Ensure that the payload is long enough to constitute"
                      " a valid ethernet frame ----------------------------");
  std::vector<uint8_t> payload{message.begin(), message.end()};

  EthMessage ethMessage;
  ethMessage.timestamp = now;
  ethMessage.ethFrame.SetSourceMac(sourceAddress);
  ethMessage.ethFrame.SetDestinationMac(destinationAddress);
  ethMessage.ethFrame.SetPayload(payload);

  ethernetController->SendMessage(ethMessage);

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


PCAP Tracing
~~~~~~~~~~~~~~~

For the Ethernet Controller, it is possible to track all received Ethernet messages in PCAP format either
in a dedicated file or to write all messages directly into a named pipe. By default, PCAP tracing is
disabled, but you can enable it in the specific settings of an Ethernet Controller (see:
:ref:`Ethernet Controller Configuration<sec:cfg-participant-ethernet>`).

PCAP File
______________

To trace all received Ethernet messages in a PCAP file, you only have to specify the according setting
in the configuration of the Ethernet Controller:

.. code-block:: javascript
    
  "EthernetControllers": [
      {
          "Name": "ETH0",
          "MacAddr": "00:08:15:ab:cd:f0",
          "PcapFile": "pcap_output_trace.pcap"
      }
  ]

After you successfully run and stopped the simulation, you can search for the file
"pcap_output_trace.pcap" and load it into a tool like
`wireshark <https://www.wireshark.org/>`_ where you can examine the Ethernet trace.

PCAP Named Pipe
_________________

In order to directly track all received Ethernet messages in a Named Pipe, you must specify
again the corresponding setting in the configuration of the Ethernet Controller:

.. code-block:: javascript
    
  "EthernetControllers": [
      {
          "Name": "ETH0",
          "MacAddr": "00:08:15:ab:cd:f0",
          "PcapPipe": "pcap_output_reader"
      }
  ]

The process responsible for the Ethernet Controller "ETH0" will try to open a Named Pipe
"pcap_output_reader" during start up of the ComAdapter. The process will be blocked until
another process connects to the Named Pipe and is ready to read any incoming Ethernet message.

The reading process could be a tool like `wireshark <https://www.wireshark.org/>`_, for example.
Under Windows, you can start wireshark in a console with a dedicated Named Pipe
and then connect to the Named Pipe by double-clicking on it::

  // Start wireshark with a dedicated Named Pipe
  wireshark -ni \\.\pipe\pcap_output_reader


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
