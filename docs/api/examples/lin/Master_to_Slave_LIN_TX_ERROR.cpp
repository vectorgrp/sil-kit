// ------------------------------------------------------------
// Slave Setup (Sender)
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

// Setup LinFrameResponseMode::TxUnconditional for LIN ID 0x10 and 0x11 on the slave
LinFrame slaveFrame;
slaveFrame.id = 0x10;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

slaveFrame.id = 0x11;
slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

// Also setup a TX Response for LIN ID 0x11 on the master (Errorneous Setup!)
LinFrame masterFrame;
masterFrame.id = 0x11;
masterFrame.dataLength = 8;
masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
masterFrame.checksumModel = LinChecksumModel::Enhanced;
masterConfig.frameResponses.push_back(LinFrameResponse{masterFrame, LinFrameResponseMode::TxUnconditional});

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent& frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);

if (UseAutosarInterface)
{
    // Transmit the frame, slave has configured a TX Response for 0x10.
    LinFrame masterFrame;
    masterFrame.id = 0x10;
    masterFrame.checksumModel = LinChecksumModel::Enhanced;
    masterFrame.dataLength = 8;
    masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};

    // AUTOSAR API
    master->SendFrame(masterFrame, LinFrameResponseType::MasterResponse);
}
else
{
    // Alternative, non-AUTOSAR API
    // Transmit the frame header, both master and slave have configured a TX Response for 0x11.
    master->SendFrameHeader(0x11);
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - LIN_TX_ERROR for the master and the slave as both provided a response for the same LIN ID.
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_TX_ERROR });
slave_FrameStatusHandler(slave, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_TX_ERROR });
