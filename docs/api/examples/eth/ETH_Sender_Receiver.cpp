// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Receiver Setup
// Register FrameHandler to receive Ethernet messages.
auto receiver_FrameHandler =
    [](IEthernetController*, const EthernetFrameEvent&) {};
ethernetReceiver->AddFrameHandler(receiver_FrameHandler);

ethernetReceiver->Activate();


// ------------------------------------------------------------
// Sender Setup
// Register FrameTransmitHandler to receive acknowledges of transmissions.
auto sender_FrameTransmitHandler =
    [](IEthernetController*, const EthernetFrameTransmitEvent&) {};
ethernetSender->AddFrameTransmitHandler(sender_FrameTransmitHandler);

ethernetSender->Activate();


// ------------------------------------------------------------
// Send an Ethernet message
std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

std::string message{"Ensure that the payload is long enough to constitute "
                    "a valid Ethernet frame ------------------------------"};
std::vector<uint8_t> payload{message.begin(), message.end()};

EthernetFrame ethFrame;
ethFrame.SetSourceMac(sourceAddress);
ethFrame.SetDestinationMac(destinationAddress);
ethFrame.SetPayload(payload);

// The returned transmitId can be used to check if the ethTransmitAcknowledge
// that should be triggered after a successful reception has the same transmitId.
auto transmitId = ethernetSender->SendFrame(ethFrame);


// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - TX confirmation for the sender.
sender_FrameTransmitHandler(ethernetSender, ethernetFrameTransmitEvent);
// with:
//  - ethTransmitAcknowledge.transmitId == 1
//  - ethTransmitAcknowledge.sourceMac == {"F6", "04", "68", "71", "AA", "C1"}
//  - ethTransmitAcknowledge.timestamp == <Timestamp of EthernetFrame>
//  - ethTransmitAcknowledge.status == EthernetTransmitStatus::Transmitted 
// Note: When using the VIBE NetworkSimulator, the status can also be EthernetTransmitStatus::LinkDown.

//  - RX Ethernet message for the receiver.
receiver_FrameHandler(ethernetReceiver, ethernetFrameEvent);
