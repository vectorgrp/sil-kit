// ------------------------------------------------------------
// Register a wake up handler. To receive wake up notifications.
// NB: this should be done when creating the controller!
// NB: receiving a wake up pulse does not automatically
//     set the controller into operational state
auto master_WakeupHandler = [](ILinController* master) { master->WakeupInternal(); };
master->RegisterWakeupHandler(master_WakeupHandler);

// ------------------------------------------------------------
// Send a wake up pulse to the LIN bus
slave->Wakeup();

// The slave will immediately enter operational state:
assert(slave->Status() == ControllerStatus::Operational);

// The master will receive the wake up pulse and the registered callback is
// triggered by the controller:
master_WakeupHandler(master);

// the registered callback puts the master back into operational state, i.e.:
assert(master->Status() == ControllerStatus::Operational);
