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
// Slave 1 Setup (Sender)
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

// Setup LinFrameResponseMode::TxUnconditional for LIN ID  0x11 on slave1
LinFrame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 1, 0, 0};
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

slave1->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave1_FrameStatusHandler = [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {
};
slave1->AddFrameStatusHandler(slave1_FrameStatusHandler);

// ------------------------------------------------------------
// Slave 2 Setup (Receiver)
LinControllerConfig slave2Config;
slave2Config.controllerMode = LinControllerMode::Slave;
slave2Config.baudRate = 20000;

// Setup LinFrameResponseMode::Rx for LIN ID 0x11 on slave2
LinFrame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::Rx});

slave2->Init(slave2Config);

// Register FrameStatusHandler to receive data
auto slave2_FrameStatusHandler = [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {
};
slave2->AddFrameStatusHandler(slave2_FrameStatusHandler);

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler = [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {
};
master->AddFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from slave to slave, i.e., slave1 provides the response, slave2 receives it.
if (UseAutosarInterface)
{
    // AUTOSAR API
    LinFrame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = LinChecksumModel::Enhanced;

    master->SendFrame(frameRequest, LinFrameResponseType::SlaveToSlave);
}
else
{
    // Alternative, non-AUTOSAR API
    master->SendFrameHeader(0x11);
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - TX confirmation for slave1, who provided the frame response
slave1_FrameStatusHandler(slave1, LinFrameStatusEvent{timeEndOfFrame, slave1Frame, LinFrameStatus::LIN_TX_OK});
//  - RX for slave2, who received the frame response
slave2_FrameStatusHandler(slave2, LinFrameStatusEvent{timeEndOfFrame, slave1Frame, LinFrameStatus::LIN_RX_OK});
