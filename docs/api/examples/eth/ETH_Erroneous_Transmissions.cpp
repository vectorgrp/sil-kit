// ------------------------------------------------------------
// Receiver Setup
ethernetReceiver->Activate();

// ------------------------------------------------------------
// Sender Setup
// Register MessageAckHandler to receive acknowledges of transmissions
auto sender_MessageAckHandler =
    [](IEthController*, const EthTransmitAcknowledge&) {};
ethernetSender->RegisterMessageAckHandler(sender_MessageAckHandler);


// ------------------------------------------------------------
// Erroneous Transmission: EthTransmitStatus::ControllerInactive
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

ethernetSender->SendMessage(ethMessage);

// The MessageAckHandler callback will be triggered and call the registered handler:
sender_MessageAckHandler(ethernetSender, ethTransmitAcknowledge);
// with ethTransmitAcknowledge.status == EthTransmitStatus::ControllerInactive


// ------------------------------------------------------------
// Erroneous Transmission: EthTransmitStatus::LinkDown
ethernetSender->Activate();
ethernetSender->SendMessage(ethMessage);

// As long as the Ethernet link is not successfully established,
// the MessageAckHandler callback will be triggered and call the registered handler:
sender_MessageAckHandler(ethernetSender, ethTransmitAcknowledge);
// with ethTransmitAcknowledge.status == EthTransmitStatus::LinkDown


// ------------------------------------------------------------
// Erroneous Transmission: EthTransmitStatus::Dropped
// Assumption: Ethernet link is already successfully established.
for (auto i = 0; i < 50; i++)
{
    ethernetSender->SendMessage(ethMessage);
}

// Sending 50 messages directly one after the other will call the registered sender_MessageAckHandler
// positively with some EthTransmitStatus::Transmitted until the transmit queue overflows
// and the Ethernet messages are acknowledged with status EthTransmitStatus::Dropped.


// ------------------------------------------------------------
// Erroneous Transmission: EthTransmitStatus::InvalidFrameFormat
std::string shortMsg{"Short message"};
std::vector<uint8_t> shortPayload{shortMsg.begin(), shortMsg.end()};

ethMessage.ethFrame.SetPayload(shortPayload);
ethernetSender->SendMessage(ethMessage);

// The MessageAckHandler callback will be triggered and call the registered handler:
sender_MessageAckHandler(ethernetSender, ethTransmitAcknowledge);
// with ethTransmitAcknowledge.status == EthTransmitStatus::InvalidFrameFormat,
// as the Ethernet frame size is too small.
