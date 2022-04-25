// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Controller Setup
// The initial state for ethernetSender is EthernetState::Inactive.
// Register StateChangeHandler to receive EthernetState changes
auto sender_StateChangedHandler =
    [](IEthernetController*, const EthernetStateChangeEvent& stateChangeEvent) {};
ethernetSender->AddStateChangeHandler(sender_StateChangedHandler);


// ------------------------------------------------------------
// Transition from EthernetState::Inactive to EthernetState::LinkDown.
ethernetSender->Activate();
// The StateChangeHandler callback will be triggered and call the registered handler:
sender_StateChangedHandler(ethernetSender, stateChangeEvent);
// with state == EthernetState::LinkDown

// ------------------------------------------------------------
// Transition from EthernetState::LinkDown to EthernetState::LinkUp.
// After some time, as soon as the link to the switch is successfully established,
// the StateChangeHandler callback will be triggered again and call the registered handler:
sender_StateChangedHandler(ethernetSender, stateChangeEvent);
// with state == EthernetState::LinkUp


// ------------------------------------------------------------
// Transition from EthernetState::LinkUp to EthernetState::Inactive.
ethernetSender->Deactivate();
// The StateChangeHandler callback will be triggered and call the registered handler:
sender_StateChangedHandler(ethernetSender, stateChangeEvent);
// with state == EthernetState::Inactive
