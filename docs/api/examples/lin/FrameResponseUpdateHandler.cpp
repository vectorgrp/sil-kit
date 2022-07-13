// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

auto master_FrameResponseUpdateHandler =
    [](ILinController*, LinFrameResponseUpdateEvent frameResponseUpdateEvent) {};
master->AddFrameResponseUpdateHandler(master_FrameResponseUpdateHandler);

// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Setup a TX Response for LIN ID 0x11
LinFrame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slave->SetFrameResponse(slaveFrame, LinFrameResponseMode::TxUnconditional);

// SetFrameResponse will result in a callback to the SlaveFrameResponseUpdateHandler:
LinFrameResponse responseUpdate;
responseUpdate.frame = slaveFrame;
responseUpdate.responseMode = LinFrameResponseMode::TxUnconditional;
master_FrameResponseUpdateHandler(master, LinFrameResponseUpdateEvent{ slaveAddress, responseUpdate });

// NB: SlaveFrameResponseUpdateHandler is not triggered when
// SetFrameResponse is called at the controller where the callback
// is registered
LinFrame masterFrame;
masterFrame.id = 0x10;
masterFrame.dataLength = 8;
masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
masterFrame.checksumModel = LinChecksumModel::Enhanced;

master->SetFrameResponse(masterFrame, LinFrameResponseMode::TxUnconditional);
// master_SlaveFrameResponseUpdate handler is not invoked.
