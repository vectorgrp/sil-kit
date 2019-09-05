// ------------------------------------------------------------
// Slave Setup
ControllerConfig slaveConfig;
slaveConfig.controllerMode = ControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive an acknowledgment for
// the successful transmission
auto slave_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
slave->RegisterFrameStatusHandler(slave_FrameStatusHandler);

// Setup a TX Response for LIN ID 0x11
Frame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};
slaveFrame.checksumModel = ChecksumModel::Enhanced;

slave->SetFrameResponse(slaveFrame, SlaveFrameResponseMode::TxUnconditional);

// ------------------------------------------------------------
// Master Setup
ControllerConfig masterConfig;
masterConfig.controllerMode = ControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive data from the LIN slave
auto master_FrameStatusHandler =
    [](ILinController*, const Frame&, FrameStatus, std::chrono::nanoseconds) {};
master->RegisterFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from slave to master, i.e., the slave provides the
// frame response, the master receives it.
if (UseAutosarInterface)
{
    // AUTOSAR API
    Frame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = ChecksumModel::Enhanced;

    master->SendFrame(frameRequest, FrameResponseType::SlaveResponse);
}
else
{
    // alternative, non-AUTOSAR API

    // 1. setup the master response
    Frame frameRequest;
    frameRequest.id = 0x11;
    frameRequest.checksumModel = ChecksumModel::Enhanced;
    master->SetFrameResponse(frameRequest, SlaveFrameResponseMode::Rx);

    // 2. transmit the frame header, the *slave* response will be transmitted automatically.
    master->SendFrameHeader(0x11);

    // Note: SendFrameHeader() can be called again without setting a new FrameResponse
}

// In both cases (AUTOSAR and non-AUTOSAR), the following callbacks will be triggered:
//  - RX for the master, who received the frame response
master_FrameStatusHandler(master, slaveFrame, FrameStatus::LIN_RX_OK, timeEndOfFrame);
//  - TX confirmation for the slave, who provided the frame response
slave_FrameStatusHandler(master, slaveFrame, FrameStatus::LIN_TX_OK, timeEndOfFrame);
