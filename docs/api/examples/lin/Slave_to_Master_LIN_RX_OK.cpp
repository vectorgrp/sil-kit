// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

// Setup LinFrameResponseMode::TxUnconditional for LIN ID 0x10 and 0x11 on the slave
LinFrame slaveFrame;
slaveFrame.id = 0x10;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 1, 0, 0};
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

slaveFrame.id = 0x11;
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive an acknowledgment for
// the successful transmission
auto slave_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

// Setup LinFrameResponseMode::Rx for LIN ID 0x11 on the master
LinFrame masterFrame;
masterFrame.id = 0x11;
masterFrame.dataLength = 8;
masterFrame.checksumModel = LinChecksumModel::Enhanced;

masterConfig.frameResponses.push_back(LinFrameResponse{masterFrame, LinFrameResponseMode::Rx});

master->Init(masterConfig);

// Register FrameStatusHandler to receive data from the LIN slave
auto master_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from slave to master, i.e., the slave provides the
// frame response, the master receives it.
if (UseAutosarInterface)
{
    // AUTOSAR API
    LinFrame frameRequest;
    frameRequest.id = 0x10;
    frameRequest.checksumModel = LinChecksumModel::Enhanced;

    master->SendFrame(frameRequest, LinFrameResponseType::SlaveResponse);
}
else
{
    // Alternative, non-AUTOSAR API
    master->SendFrameHeader(0x11);
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - RX for the master, who received the frame response
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, slaveFrame, LinFrameStatus::LIN_RX_OK });
//  - TX confirmation for the slave, who provided the frame response
slave_FrameStatusHandler(slave, LinFrameStatusEvent{ timeEndOfFrame, slaveFrame, LinFrameStatus::LIN_TX_OK });
