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
// Slave 2 Setup (Receiver)
ControllerConfig slave2Config;
slave2Config.controllerMode = ControllerMode::Slave;
slave2Config.baudRate = 20000;

slave2->Init(slave2Config);

// Register FrameStatusHandler to receive data
auto slave2_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave2->RegisterFrameStatusHandler(slave2_FrameStatusHandler);

// Setup LIN ID 0x11 as RX
Frame slave2Frame;
slave2Frame.id = 0x11;
slave2Frame.checksumModel = ChecksumModel::Enhanced;
slave2Frame.dataLength = 8;

slave2->SetFrameResponse(slave2Frame, FrameResponseMode::Rx);

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
// Perform TX from slave to slave, i.e., slave1 provides the response, slave2 receives it.
if (UseAutosarInterface)
{
    // AUTOSAR API
    Frame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = ChecksumModel::Enhanced;

    master->SendFrame(frameRequest, FrameResponseType::SlaveToSlave);
}
else
{
    // alternative, non-AUTOSAR API

    // 1. setup the master response
    Frame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = ChecksumModel::Enhanced;
    master->SetFrameResponse(frameRequest, FrameResponseMode::Unused);

    // 2. transmit the frame header, the *slave* response will be transmitted automatically.
    master->SendFrameHeader(0x11);

    // Note: SendFrameHeader() can be called again without setting a new FrameResponse
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - TX confirmation for the master, who initiated the slave to slave transmission
master_FrameStatusHandler(master, slave1Frame, FrameStatus::LIN_TX_OK, timeEndOfFrame);
//  - TX confirmation for slave1, who provided the frame response
slave1_FrameStatusHandler(slave1, slave1Frame, FrameStatus::LIN_TX_OK, timeEndOfFrame);
//  - RX for slave2, who received the frame response
slave2_FrameStatusHandler(slave2, slave1Frame, FrameStatus::LIN_RX_OK, timeEndOfFrame);
