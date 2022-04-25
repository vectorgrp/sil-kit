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
std::array<uint8_t, 6> sourceAddress{"F6", "04", "68", "71", "AA", "C1"};
std::array<uint8_t, 6> destinationAddress{"F6", "04", "68", "71", "AA", "C2"};

std::string message{"Ensure that the payload is long enough to constitute "
                    "a valid Ethernet frame ------------------------------"};
std::vector<uint8_t> payload{message.begin(), message.end()};

EthernetFrame ethFrame;
ethFrame.SetSourceMac(sourceAddress);
ethFrame.SetDestinationMac(destinationAddress);
ethFrame.SetPayload(payload);

ethernetSender->SendFrame(ethFrame);

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
    ethernetSender->SendFrame(ethFrame);
}

// Sending 50 messages directly one after the other will call the registered sender_MessageAckHandler
// positively with some EthernetTransmitStatus::Transmitted until the transmit queue overflows
// and the Ethernet messages are acknowledged with status EthernetTransmitStatus::Dropped.


// ------------------------------------------------------------
// Erroneous Transmission: EthernetTransmitStatus::InvalidFrameFormat
std::string shortMsg{"Short message"};
std::vector<uint8_t> shortPayload{shortMsg.begin(), shortMsg.end()};

ethFrame.SetPayload(shortPayload);
ethernetSender->SendFrame(ethFrame);

// The MessageAckHandler callback will be triggered and call the registered handler:
sender_FrameTransmitHandler(ethernetSender, frameTransmitEvent);
// with frameTransmitEvent.status == EthernetTransmitStatus::InvalidFrameFormat,
// as the Ethernet frame size is too small.
