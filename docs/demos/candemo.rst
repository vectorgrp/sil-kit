Can Controllers
===============

Any Simulation

* CAN messages are sent using ICanController::SendMessage(...).
* Received CAN messages trigger the callback that was registered using ICanController::RegisterReceiveMessageHandler(...).

Simple Simulation

* CAN messages can be directly sent and received without any further configuration of the controller.
* Transmits CAN message regardless of content (CAN id, flags, ...).
* No need to configure or start the CanController Only ReceiveMessageHandler and TransmitStatusHandler are supported.
* TransmitStatusHandler will either receive an Acknowledge if there is at least one other CanController or won't be called at all.
* There is no error indication.
* No support for StateChangedHandler and ErrorStateHandler.
* Time stamps must be set manually.

VIBE Simulation

* Full support for all configuration methods and callbacks.
* Baud rate must be set and CanController must be started before CAN messages can be sent.


Explanation based on the CAN Demo
---------------------------------

FIXME:

The CAN acknowledges and CAN messages are received over the following Callbacks:

.. code-block:: cpp

    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);
    auto* canController = comAdapter->CreateCanController("CAN1");

    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
