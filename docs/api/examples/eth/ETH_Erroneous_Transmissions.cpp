// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Receiver Setup
ethernetReceiver->Activate();

// ------------------------------------------------------------
// Sender Setup
// Register MessageAckHandler to receive acknowledges of transmissions
auto sender_FrameTransmitHandler =
    [](IEthernetController*, const EthernetFrameTransmitEvent&) {};
ethernetSender->AddFrameTransmitHandler(sender_FrameTransmitHandler);

// ------------------------------------------------------------
// Erroneous Transmission: EthernetTransmitStatus::ControllerInactive
const std::array<uint8_t, 6> sourceAddress{0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc2};
const std::array<uint8_t, 6> destinationAddress{0xf6, 0x04, 0x68, 0x71, 0xaa, 0xc1};
const std::array<uint8_t, 2> etherType{0x08, 0x00};

const std::string message{"Ensure that the payload is at least 46 bytes to constitute "
                    "a valid Ethernet frame ------------------------------"};
const std::vector<uint8_t> payload{ message.begin(), message.end() };

EthernetFrame frame;
std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(frame));
std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(frame));
std::copy(etherType.begin(), etherType.end(), std::back_inserter(frame));
std::copy(payload.begin(), payload.end(), std::back_inserter(frame));

ethernetSender->SendFrame(frame);

// The FrameTransmitHandler callback will be triggered and call the registered handler:
sender_FrameTransmitHandler(ethernetSender, frameTransmitEvent);
// with frameTransmitEvent.status == EthernetTransmitStatus::ControllerInactive


// ------------------------------------------------------------
// Erroneous Transmission: EthernetTransmitStatus::LinkDown
ethernetSender->Activate();
ethernetSender->SendFrame(ethernetFrame);

// As long as the Ethernet link is not successfully established,
// the MessageAckHandler callback will be triggered and call the registered handler:
sender_FrameTransmitHandler(ethernetSender, frameTransmitEvent);
// with frameTransmitEvent.status == EthernetTransmitStatus::LinkDown

// ------------------------------------------------------------
// Erroneous Transmission: EthernetTransmitStatus::Dropped
// Assumption: Ethernet link is already successfully established.
for (auto i = 0; i < 50; i++)
{
    ethernetSender->SendFrame(ethernetFrame);
}

// Sending 50 messages directly one after the other will call the registered sender_MessageAckHandler
// positively with some EthernetTransmitStatus::Transmitted until the transmit queue overflows
// and the Ethernet messages are acknowledged with status EthernetTransmitStatus::Dropped.


// ------------------------------------------------------------
// Erroneous Transmission: EthernetTransmitStatus::InvalidFrameFormat
const std::string shortMessage{"A payload with less than 46 bytes is invalid."};
const std::vector<uint8_t> shortPayload{shortMessage.begin(), shortMessage.end()};

EthernetFrame invalidFrame;
std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(invalidFrame));
std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(invalidFrame));
std::copy(etherType.begin(), etherType.end(), std::back_inserter(invalidFrame));
std::copy(shortPayload.begin(), shortPayload.end(), std::back_inserter(invalidFrame));

ethernetSender->SendFrame(invalidEthernetFrame);

// The MessageAckHandler callback will be triggered and call the registered handler:
sender_FrameTransmitHandler(ethernetSender, frameTransmitEvent);
// with frameTransmitEvent.status == EthernetTransmitStatus::InvalidFrameFormat,
// as the Ethernet frame size is too small.
