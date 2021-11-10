// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Receiver Setup
// Register ReceiveMessageHandler to receive Ethernet messages.
auto receiver_ReceiveMessageHandler =
    [](IEthController*, const EthMessage&) {};
ethernetReceiver->RegisterReceiveMessageHandler(receiver_ReceiveMessageHandler);

ethernetReceiver->Activate();


// ------------------------------------------------------------
// Sender Setup
// Register MessageAckHandler to receive acknowledges of transmissions.
auto sender_MessageAckHandler =
    [](IEthController*, const EthTransmitAcknowledge&) {};
ethernetSender->RegisterMessageAckHandler(sender_MessageAckHandler);

ethernetSender->Activate();


// ------------------------------------------------------------
// Send an Ethernet message
std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

std::string message{"Ensure that the payload is long enough to constitute "
                    "a valid ethernet frame ------------------------------"};
std::vector<uint8_t> payload{message.begin(), message.end()};

EthMessage ethMessage;
ethMessage.timestamp = now;
ethMessage.ethFrame.SetSourceMac(sourceAddress);
ethMessage.ethFrame.SetDestinationMac(destinationAddress);
ethMessage.ethFrame.SetPayload(payload);

// The returned transmitId can be used to check if the ethTransmitAcknowledge
// that should be triggerd after a successful reception has the same transmitId.
auto transmitId = ethernetSender->SendMessage(ethMessage);


// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - TX confirmation for the sender.
sender_MessageAckHandler(ethernetSender, ethTransmitAcknowledge);
// with:
//  - ethTransmitAcknowledge.transmitId == 1
//  - ethTransmitAcknowledge.sourceMac == {"F6", "04", "68", "71", "AA", "C1"}
//  - ethTransmitAcknowledge.timestamp == <Timestamp of EthernetFrame>
//  - ethTransmitAcknowledge.status == EthTransmitStatus::Transmitted 
// Note: When using the VIBE NetworkSimulator, the status can also be EthTransmitStatus::LinkDown.

//  - RX Ethernet message for the receiver.
receiver_ReceiveMessageHandler(ethernetReceiver, ethMessage);
