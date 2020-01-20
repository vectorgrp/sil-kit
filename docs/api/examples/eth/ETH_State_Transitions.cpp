// ------------------------------------------------------------
// Controller Setup
// The initial state for ethernetSender is EthState::Inactive.
// Register StateChangedHandler to receive EthState changes
auto sender_StateChangedHandler =
    [](IEthController*, const EthState& state) {};
ethernetSender->RegisterStateChangedHandler(sender_StateChangedHandler);


// ------------------------------------------------------------
// Transition from EthState::Inactive to EthState::LinkDown.
ethernetSender->Activate();
// The StateChangedHandler callback will be triggered and call the registered handler:
sender_StateChangedHandler(ethernetSender, state);
// with state == EthState::LinkDown

// ------------------------------------------------------------
// Transition from EthState::LinkDown to EthState::LinkUp.
// After some time, as soon as the link to the switch is successfully established,
// the StateChangedHandler callback will be triggered again and call the registered handler:
sender_StateChangedHandler(ethernetSender, state);
// with state == EthState::LinkUp


// ------------------------------------------------------------
// Transition from EthState::LinkUp to EthState::Inactive.
ethernetSender->Deactivate();
// The StateChangedHandler callback will be triggered and call the registered handler:
sender_StateChangedHandler(ethernetSender, state);
// with state == EthState::Inactive
