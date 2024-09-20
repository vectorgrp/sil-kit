/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
const std::string longMessage(4000, 'a'); // much longer than the maximum allowed Ethernet frame size of 1534 bytes
const std::vector<uint8_t> longPayload{longMessage.begin(), longMessage.end()};

EthernetFrame invalidFrame;
std::copy(destinationAddress.begin(), destinationAddress.end(), std::back_inserter(invalidFrame));
std::copy(sourceAddress.begin(), sourceAddress.end(), std::back_inserter(invalidFrame));
std::copy(etherType.begin(), etherType.end(), std::back_inserter(invalidFrame));
std::copy(longPayload.begin(), longPayload.end(), std::back_inserter(invalidFrame));

ethernetSender->SendFrame(invalidEthernetFrame);

// The MessageAckHandler callback will be triggered and call the registered handler:
sender_FrameTransmitHandler(ethernetSender, frameTransmitEvent);
// with frameTransmitEvent.status == EthernetTransmitStatus::InvalidFrameFormat,
// as the Ethernet frame size is too long.
