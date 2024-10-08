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
