// ------------------------------------------------------------
// Receiver Setup
canReceiver->SetBaudRate(10000, 1000000);
canReceiver->Start();

// Register CanFrameHandler to receive data
auto receiver_frameHandler = [](ICanController*, const CanFrameEvent& frameEvent) {};
canReceiver->AddFrameHandler(receiver_frameHandler);

// ------------------------------------------------------------
// Sender Setup
canSender->SetBaudRate(10000, 1000000);
canSender->Start();

// Register FrameTransmitHandler to receive acknowledge of the successful transmission
auto sender_frameTransmitHandler = [](ICanController*, const CanFrameTransmitEvent& frameTransmitEvent) {};
canSender->AddFrameTransmitHandler(sender_frameTransmitHandler);

// ------------------------------------------------------------
// Send message on CAN bus "CAN1".
CanFrame canFrame;
canFrame.canId = 17;
canFrame.flags.ide = 0; // Identifier Extension
canFrame.flags.rtr = 0; // Remote Transmission Request
canFrame.flags.fdf = 0; // FD Format Indicator
canFrame.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
canFrame.flags.esi = 0; // Error State indicator (for FD Format only)
canFrame.dataField = {'d', 'a', 't', 'a', 0, 1, 2, 3};
canFrame.dlc = canMessage.dataField.size();

// The returned transmitId can be used to check if the canTransmitAcknowledge
// that should be triggered after a successful reception has the same transmitId
auto transmitId = canSender->SendFrame(canFrame);

// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - TX confirmation for the sender. Only triggered once independently of the receiver count.
sender_frameTransmitHandler(canSender, frameTransmitEvent);
// with:
//  - frameTransmitEvent.transmitId == 1
//  - frameTransmitEvent.canId == 17
//  - frameTransmitEvent.timestamp = 
//  - frameTransmitEvent.status == CanTransmitStatus.Transmitted
//
//  - RX for the receiver or any other controller that is connected to the same CAN bus
//    except the sender of the CAN message
receiver_frameHandler(canReceiver, frameEvent);
