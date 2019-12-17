// ------------------------------------------------------------
// Receiver Setup
canReceiver->SetBaudRate(10000, 1000000);
canReceiver->Start();

// Register ReceiveMessageHandler to receive data
auto receiver_ReceiveMessageHandler =
    [](ICanController*, const CanMessage&) {};
canReceiver->RegisterReceiveMessageHandler(receiver_ReceiveMessageHandler);


// ------------------------------------------------------------
// Sender Setup
canSender->SetBaudRate(10000, 1000000);
canSender->Start();

// Register TransmitStatusHandler to receive acknowledge of the successful transmission
auto sender_ReceiveTransmitStatusHandler =
    [](ICanController*, const CanTransmitAcknowledge&) {};
canSender->RegisterTransmitStatusHandler(sender_ReceiveTransmitStatusHandler);


// ------------------------------------------------------------
// Send message on CAN bus "CAN1".
CanMessage canMessage;
canMessage.timestamp = now;
canMessage.canId = 17;
canMessage.flags.ide = 0; // Identifier Extension
canMessage.flags.rtr = 0; // Remote Transmission Request
canMessage.flags.fdf = 0; // FD Format Indicator
canMessage.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
canMessage.flags.esi = 0; // Error State indicator (for FD Format only)
canMessage.dataField = {'d', 'a', 't', 'a', 0, 1, 2, 3};
canMessage.dlc = canMessage.dataField.size();

// The returned transmitId can be used to check if the canTransmitAcknowledge
// that should be triggerd after a successful reception has the same transmitId
auto transmitId = canSender->SendMessage(canMessage);


// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - TX confirmation for the sender. Only triggered once independently of the receiver count.
sender_ReceiveTransmitStatusHandler(canSender, canTransmitAcknowledge);
// with:
//  - canTransmitAcknowledge.transmitId == 1
//  - canTransmitAcknowledge.canId == 17
//  - canTransmitAcknowledge.timestamp = 
//  - canTransmitAcknowledge.status == CanTransmitStatus.Transmitted
//
//  - RX for the receiver or any other controller that is connected to the same CAN bus
//    except the sender of the CAN message
receiver_ReceiveMessageHandler(canReceiver, canMessage);
