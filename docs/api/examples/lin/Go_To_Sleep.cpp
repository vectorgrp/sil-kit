// ------------------------------------------------------------
// Slave Setup
LinControllerConfig slaveConfig;
slaveConfig.controllerMode = LinControllerMode::Slave;
slaveConfig.baudRate = 20000;

slave->Init(slaveConfig);

// Register FrameStatusHandler to receive data
auto slave_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent frameStatusEvent) {};
slave->AddFrameStatusHandler(slave_FrameStatusHandler);

// Set sleep mode for the slave upon reception of a GoToSleep Frame
auto slave_GoToSleepHandler =
    [](ILinController* slave, const LinGoToSleepEvent& goToSleepEvent) { 
    slave->GoToSleepInternal(); 
};
slave->AddGoToSleepHandler(slave_GoToSleepHandler);

// ------------------------------------------------------------
// Master Setup
LinControllerConfig masterConfig;
masterConfig.controllerMode = LinControllerMode::Master;
masterConfig.baudRate = 20000;

master->Init(masterConfig);

// Register FrameStatusHandler to receive confirmation of the successful transmission
auto master_FrameStatusHandler =
    [](ILinController*, const LinFrameStatusEvent frameStatusEvent) {};
master->AddFrameStatusHandler(master_FrameStatusHandler);


// ------------------------------------------------------------
// Send a GoToSleep Frame to the LIN bus
master->GoToSleep();

// The master will enter sleep state immediately, i.e., the following condition is true:
assert(master->Status() == LinControllerStatus::Sleep);

// The slave will receive the go-to-sleep frame and trigger the callback:
slave_GoToSleepHandler(goToSleepEvent);

// the registered callback sets sleep state for the slave, after which also the slave is in sleep state:
assert(master->Status() == LinControllerStatus::Sleep);
