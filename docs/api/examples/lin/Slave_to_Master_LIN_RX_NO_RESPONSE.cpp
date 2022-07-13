// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler
auto slave_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// NOTE: No TX response is configured for the slave

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive data from the LIN slave
auto master_FrameStatusHandler =
    [](ILinController*,  const LinFrameStatusEvent& frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);

// ------------------------------------------------------------
// Perform TX from slave to master, i.e., the slave /is expected/
// to provide the frame response.
LinFrame frameRequest;
frameRequest.id = 0x11;
frameRequest.checksumModel = LinChecksumModel::Enhanced;

// Use AUTOSAR interface to initiate the transmission.
master->SendFrame(frameRequest, LinFrameResponseType::SlaveResponse);

// ------------------------------------------------------------
// The following master callback will be triggered:
//  - LIN_RX_NO_RESPONSE for the master, since no slave did provide a response
master_FrameStatusHandler(master, LinFrameStatusEvent{ timeEndOfFrame, frameRequest, LinFrameStatus::LIN_RX_NO_RESPONSE });
//  The slave_FrameStatusHandler will not be called!
