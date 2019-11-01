// ------------------------------------------------------------
// Slave 1 Setup (Sender)
ControllerConfig slaveConfig;
slaveConfig.controllerMode = ControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave1->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave1_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave1->RegisterFrameStatusHandler(slave1_FrameStatusHandler);

// Setup a TX Response for LIN ID 0x11
Frame slave1Frame;
slave1Frame.id = 0x11;
slave1Frame.checksumModel = ChecksumModel::Enhanced;
slave1Frame.dataLength = 8;
slave1Frame.data = {'S', 'L', 'A', 'V', 'E', '1', 0, 0};

slave1->SetFrameResponse(slave1Frame, FrameResponseMode::TxUnconditional);

// ------------------------------------------------------------
// Slave 2 Setup (Second Sender)
ControllerConfig slave2Config;
slave2Config.controllerMode = ControllerMode::Slave;
slave2Config.baudRate = 20000;

slave2->Init(slave2Config);

// Register FrameStatusHandler to receive data
auto slave2_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave2->RegisterFrameStatusHandler(slave2_FrameStatusHandler);

// Also setup a TX Response for LIN ID 0x11
Frame slave2Frame;
slave2Frame.id = 0x11;
slave2Frame.checksumModel = ChecksumModel::Enhanced;
slave2Frame.dataLength = 8;
slave2Frame.data = {'S', 'L', 'A', 'V', 'E', '2', 0, 0};

slave2->SetFrameResponse(slave2Frame, FrameResponseMode::TxUnconditional);

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
// Perform TX from slave to master, i.e., only one slave /is/
// /expected/ to provide the frame response. However, both
// slave1 and slave2 have done so.
Frame frameRequest;
frameRequest.id = 0x11;
frameRequest.checksumModel = ChecksumModel::Enhanced;

// Use AUTOSAR interface to initiate the transmission.
master->SendFrame(frameRequest, FrameResponseType::SlaveResponse);

// ------------------------------------------------------------
// The following callbacks will be triggered:
//  - LIN_RX_ERROR for the master, due to the collision 
master_FrameStatusHandler(master, frameRequest, FrameStatus::LIN_RX_ERROR, timeEndOfFrame);
//  - LIN_TX_ERROR for both slaves
slave1_FrameStatusHandler(slave1, frameRequest, FrameStatus::LIN_TX_ERROR, timeEndOfFrame);
slave2_FrameStatusHandler(slave2, frameRequest, FrameStatus::LIN_TX_ERROR, timeEndOfFrame);
