// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Master Setup
ControllerConfig masterConfig;
masterConfig.controllerMode = ControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

auto master_FrameResponseUpdateHandler =
    [](ILinController*, mw::EndpointAddress, const FrameResponse&) {};
master->RegisterFrameResponseUpdateHandler(master_FrameResponseUpdateHandler);

// ------------------------------------------------------------
// Slave Setup
ControllerConfig slaveConfig;
slaveConfig.controllerMode = ControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Setup a TX Response for LIN ID 0x11
Frame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = ChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slave->SetFrameResponse(slaveFrame, FrameResponseMode::TxUnconditional);

// SetFrameResponse will result in a callback to the SlaveFrameResponseUpdateHandler:
FrameResponse responseUpdate;
responseUpdate.frame = slaveFrame;
responseUpdate.responseMode = FrameResponseMode::TxUnconditional;
master_FrameResponseUpdateHandler(master, slaveAddress, responseUpdate);


// NB: SlaveFrameResponseUpdateHandler is not triggered when
// SetFrameResponse is called at the controller where the callback
// is registered
Frame masterFrame;
masterFrame.id = 0x10;
masterFrame.dataLength = 8;
masterFrame.data = {'M', 'A', 'S', 'T', 'E', 'R', 0, 0};
masterFrame.checksumModel = ChecksumModel::Enhanced;

master->SetFrameResponse(masterFrame, FrameResponseMode::TxUnconditional);
// master_SlaveFrameResponseUpdate handler is not invoked.
