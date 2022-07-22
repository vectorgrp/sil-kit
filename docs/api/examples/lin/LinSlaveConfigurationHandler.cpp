// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;
master->Init(masterConfig);

// Define the LinSlaveConfigurationHandler
auto master_LinSlaveConfigurationHandler =
    [](ILinController* master, LinSlaveConfigurationEvent frameResponseUpdateEvent) 
{
    // aggregatedSlaveConfig will contain
    // LinSlaveConfiguration::respondingLinIds{ 0x11 }
    auto aggregatedSlaveConfig = master->GetSlaveConfiguration();
};
master->AddLinSlaveConfigurationHandler(master_LinSlaveConfigurationHandler);

// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

// Setup a TX Response for LIN ID 0x11
LinFrame slaveFrame;
slaveFrame.id = 0x11;
slaveFrame.checksumModel = LinChecksumModel::Enhanced;
slaveFrame.dataLength = 8;
slaveFrame.data = {'S', 'L', 'A', 'V', 'E', 0, 0, 0};

slaveConfig.frameResponses.push_back(LinFrameResponse{slaveFrame, LinFrameResponseMode::TxUnconditional});

// master_LinSlaveConfigurationHandler handler is invoked
slave->Init(slaveConfig);
