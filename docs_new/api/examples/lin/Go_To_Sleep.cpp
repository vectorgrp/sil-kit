// Copyright (c) Vector Informatik GmbH. All rights reserved.
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

// Set sleep mode for the slave upon reception of a GoToSleep Frame
auto slave_GoToSleepHandler =
    [](ILinController* slave) { slave->GoToSleepInternal(); };
slave->RegisterGoToSleepHandler(slave_GoToSleepHandler);

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
// Send a GoToSleep Frame to the LIN bus
master->GoToSleep();

// The master will enter sleep state immediately, i.e., the following condition is true:
assert(master->Status() == ControllerStatus::Sleep);

// The slave will receive the go-to-sleep frame and trigger the callback:
slave_GoToSleepHandler(slave);

// the registered callback sets sleep state for the slave, after which also the slave is in sleep state:
assert(master->Status() == ControllerStatus::Sleep);
