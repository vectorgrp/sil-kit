Can Demo
========

Code Explanation
----------------

The CAN acknowledges and CAN messages are received over the following Callbacks:

.. code-block:: cpp

    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);
    auto* canController = comAdapter->CreateCanController("CAN1");

    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
