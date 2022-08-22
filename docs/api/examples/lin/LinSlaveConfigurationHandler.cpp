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
// 
// Master Setup
using namespace SilKit::Services::Lin;
using namespace SilKit::Experimental::Services::Lin;

LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;
master->Init(masterConfig);

// Define the LinSlaveConfigurationHandler
auto LinSlaveConfigurationHandler =
    [](ILinController* master, LinSlaveConfigurationEvent frameResponseUpdateEvent) 
{
    // aggregatedSlaveConfig will contain
    // LinSlaveConfiguration::respondingLinIds{ 0x11 }
    auto aggregatedSlaveConfig = GetSlaveConfiguration(master);
};

AddLinSlaveConfigurationHandler(master, &LinSlaveConfigurationHandler);

// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

// Setup a TX Response for LIN ID 0x11
LinFrame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

// LinSlaveConfigurationHandler handler is invoked
slave->Init(slaveConfig);
