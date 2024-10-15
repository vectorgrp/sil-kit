/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
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
assert(slave->Status() == LinControllerStatus::Sleep);
