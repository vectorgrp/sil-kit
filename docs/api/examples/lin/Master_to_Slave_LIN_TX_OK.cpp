// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// Setup ID 0x10 for reception
LinFrame rxFrame;
rxFrame.id = 0x10;
rxFrame.checksumModel = LinChecksumModel::Enhanced;
rxFrame.dataLength = 8;

slave->SetFrameResponse(rxFrame, LinFrameResponseMode::Rx);

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
// Perform TX from master to slave, i.e., the master provides the
// frame response.
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
//  - TX confirmation for the master, who initiated the transmission
//    and provided the frame response
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_TX_OK });
//  - RX for the slave, who received the frame response
slave_FrameStatusHandler(slave, LinFrameStatusEvent{ timeEndOfFrame, masterFrame, LinFrameStatus::LIN_RX_OK });
