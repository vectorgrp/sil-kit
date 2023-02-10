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
canReceiver->SetBaudRate(10000, 1000000, 1000000);
canReceiver->Start();

// Register CanFrameHandler to receive data
auto receiver_frameHandler = [](ICanController*, const CanFrameEvent& frameEvent) {};
canReceiver->AddFrameHandler(receiver_frameHandler);

// ------------------------------------------------------------
// Sender Setup
canSender->SetBaudRate(10000, 1000000, 1000000);
canSender->Start();

// Register FrameTransmitHandler to receive acknowledge of the successful transmission
auto sender_frameTransmitHandler = [](ICanController*, const CanFrameTransmitEvent& frameTransmitEvent) {};
canSender->AddFrameTransmitHandler(sender_frameTransmitHandler);

// ------------------------------------------------------------
// Send message on CAN bus "CAN1".
const std::vector<uint8_t> canFrameData = {'d', 'a', 't', 'a', 0, 1, 2, 3};
CanFrame canFrame{};
canFrame.canId = 17;
canFrame.flags =
    // FD Format Indicator
      static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)
    // Bit Rate Switch  (for FD Format only)
    | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs);
canFrame.dataField = canFrameData;
canFrame.dlc = canFrame.dataField.size();

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
