// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Slave Setup (Sender)
ControllerConfig slaveConfig;
slaveConfig.controllerMode = ControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave->RegisterFrameStatusHandler(slave_FrameStatusHandler);

// Setup a TX Response for LIN ID 0x10
Frame slaveFrame;
slaveFrame.id = 0x10;
slaveFrame.checksumModel = ChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slave->SetFrameResponse(slaveFrame, FrameResponseMode::TxUnconditional);


// ------------------------------------------------------------
// Master Setup
ControllerConfig masterConfig;
masterConfig.controllerMode = ControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
master->RegisterFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from master to slave for LIN ID 0x10, i.e., the master also
// provides a frame response for the same LIN ID.
Frame masterFrame;
masterFrame.id = 0x10;
masterFrame.checksumModel = ChecksumModel::Enhanced;
masterFrame.dataLength = 8;
masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};

if (UseAutosarInterface)
{
    // AUTOSAR API
    master->SendFrame(masterFrame, FrameResponseType::MasterResponse);
}
else
{
    // alternative, non-AUTOSAR API

    // 1. setup the master response
    master->SetFrameResponse(masterFrame, FrameResponseMode::TxUnconditional);
    // 2. transmit the frame header, the master response will be transmitted automatically.
    master->SendFrameHeader(0x10);

    // Note: SendFrameHeader() can be called again without setting a new FrameResponse
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - LIN_TX_ERROR for the master and the slave as both provided a response for the same LIN ID.
master_FrameStatusHandler(master, masterFrame, FrameStatus::LIN_TX_ERROR, timeEndOfFrame);
slave_FrameStatusHandler(slave, masterFrame, FrameStatus::LIN_TX_ERROR, timeEndOfFrame);
