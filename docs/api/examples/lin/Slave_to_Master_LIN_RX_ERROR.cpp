// ------------------------------------------------------------
// Slave 1 Setup (Sender)
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave1->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave1_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
slave1->AddFrameStatusHandler(slave1_FrameStatusHandler);

// Setup a TX Response for LIN ID 0x11
LinFrame slave1Frame;
slave1Frame.id = 0x11;
slave1Frame.checksumModel = LinChecksumModel::Enhanced;
slave1Frame.dataLength = 8;
slave1Frame.data = {'S', 'L', 'A', 'V', 'E', '1', 0, 0};

slave1->SetFrameResponse(slave1Frame, LinFrameResponseMode::TxUnconditional);

// ------------------------------------------------------------
// Slave 2 Setup (Second Sender)
LinControllerConfig slave2Config;
slave2Config.controllerMode = LinControllerMode::Slave;
slave2Config.baudRate = 20000;

slave2->Init(slave2Config);

// Register FrameStatusHandler to receive data
auto slave2_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
slave2->AddFrameStatusHandler(slave2_FrameStatusHandler);

// Also setup a TX Response for LIN ID 0x11
LinFrame slave2Frame;
slave2Frame.id = 0x11;
slave2Frame.checksumModel = LinChecksumModel::Enhanced;
slave2Frame.dataLength = 8;
slave2Frame.data = {'S', 'L', 'A', 'V', 'E', '2', 0, 0};

slave2->SetFrameResponse(slave2Frame, LinFrameResponseMode::TxUnconditional);

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from slave to master, i.e., only one slave /is/
// /expected/ to provide the frame response. However, both
// slave1 and slave2 have done so.
LinFrame frameRequest;
frameRequest.id = 0x11;
frameRequest.checksumModel = LinChecksumModel::Enhanced;

// Use AUTOSAR interface to initiate the transmission.
master->SendFrame(frameRequest, LinFrameResponseType::SlaveResponse);

// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - LIN_RX_ERROR for the master, due to the collision 
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, frameRequest, LinFrameStatus::LIN_RX_ERROR });
//  - LIN_TX_ERROR for both slaves
slave1_FrameStatusHandler(slave1, LinFrameStatusEvent{timeEndOfFrame, frameRequest, LinFrameStatus::LIN_TX_ERROR});
slave2_FrameStatusHandler(slave2, LinFrameStatusEvent{timeEndOfFrame, frameRequest, LinFrameStatus::LIN_TX_ERROR});
