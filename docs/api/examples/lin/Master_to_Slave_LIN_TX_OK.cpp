// ------------------------------------------------------------
// Slave Setup
ControllerConfig slaveConfig;
slaveConfig.controllerMode = ControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave->RegisterFrameStatusHandler(slave_FrameStatusHandler);

// Setup ID 0x10 for reception
Frame rxFrame;
rxFrame.id = 0x10;
rxFrame.checksumModel = ChecksumModel::Enhanced;
rxFrame.dataLength = 8;

slave->SetFrameResponse(rxFrame, FrameResponseMode::Rx);

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
// Perform TX from master to slave, i.e., the master provides the
// frame response.
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
//  - TX confirmation for the master, who initiated the transmission
//    and provided the frame response
master_FrameStatusHandler(master, masterFrame, FrameStatus::LIN_TX_OK, timeEndOfFrame);
//  - RX for the slave, who received the frame response
slave_FrameStatusHandler(master, masterFrame, FrameStatus::LIN_RX_OK, timeEndOfFrame);
