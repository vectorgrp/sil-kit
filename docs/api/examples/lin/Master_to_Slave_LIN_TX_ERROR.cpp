// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Slave Setup (Sender)
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// Setup a TX Response for LIN ID 0x10
LinFrame slaveFrame;
slaveFrame.id = 0x10;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slave->SetFrameResponse(slaveFrame, LinFrameResponseMode::TxUnconditional);


// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from master to slave for LIN ID 0x10, i.e., the master also
// provides a frame response for the same LIN ID.
LinFrame masterFrame;
masterFrame.id = 0x10;
masterFrame.checksumModel = LinChecksumModel::Enhanced;
masterFrame.dataLength = 8;
masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};

if (UseAutosarInterface)
{
    // AUTOSAR API
    master->SendFrame(masterFrame, LinFrameResponseType::MasterResponse);
}
else
{
    // alternative, non-AUTOSAR API

    // 1. setup the master response
    master->SetFrameResponse(masterFrame, LinFrameResponseMode::TxUnconditional);
    // 2. transmit the frame header, the master response will be transmitted automatically.
    master->SendFrameHeader(0x10);

    // Note: SendFrameHeader() can be called again without setting a new LinFrameResponse
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - LIN_TX_ERROR for the master and the slave as both provided a response for the same LIN ID.
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_TX_ERROR });
slave_FrameStatusHandler(slave, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_TX_ERROR });
