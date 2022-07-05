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
const std::array<uint8_t, 6> destinationAddress{0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc2};
const std::array<uint8_t, 6> sourceAddress{0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc1};
const std::array<uint8_t, 4> vlanTag{0x81, 0x00, 0x00, 0x00};  // optional
const std::array<uint8_t, 2> etherType{0x08, 0x00};
const std::array<uint8_t, 4> frameCheckSequence{0x00, 0x00, 0x00, 0x00};  // optional

const std::string message{"Ensure that the payload is at least 46 bytes to constitute "
                    "a valid Ethernet frame ------------------------------"};
const std::vector<uint8_t> payload{message.begin(), message.end()};

const std::array<uint8_t, 2> frameCheckSequence{0x00, 0x00};  // optional for 

EthernetFrame frame{};
std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(frame));
std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(frame));
std::copy(vlanTag.begin(), vlanTag.end(), std::back_inserter(frame));
std::copy(etherType.begin(), etherType.end(), std::back_inserter(frame));
std::copy(payload.begin(), payload.end(), std::back_inserter(frame));
std::copy(frameCheckSequence.begin(), frameCheckSequence.end(), std::back_inserter(frame));

// The returned transmitId can be used to check if the ethernetFrameTransmitEvent
// that should be triggered after a successful reception has the same transmitId.
auto transmitId = ethernetSender->SendFrame(frame);


// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - TX confirmation for the sender.
sender_FrameTransmitHandler(ethernetSender, ethernetFrameTransmitEvent);
// with:
//  - ethernetFrameTransmitEvent.transmitId == 1
//  - ethernetFrameTransmitEvent.sourceMac == {0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1}
//  - ethernetFrameTransmitEvent.timestamp == <Timestamp of EthernetFrame>
//  - ethernetFrameTransmitEvent.status == EthernetTransmitStatus::Transmitted 
// Note: In a detailed simulation, the status can also be EthernetTransmitStatus::LinkDown.

//  - RX Ethernet message for the receiver.
receiver_FrameHandler(ethernetReceiver, ethernetFrameEvent);
