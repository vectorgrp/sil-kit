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
// Register a wake up handler. To receive wake up notifications.
// NB: this should be done when creating the controller!
// NB: receiving a wake up pulse does not automatically
//     set the controller into operational state
auto master_WakeupHandler = [](ILinController* master, LinWakeupEvent wakeupEvent) { master->WakeupInternal(); };
master->AddWakeupHandler(master_WakeupHandler);

// ------------------------------------------------------------
// Send a wake up pulse to the LIN bus
slave->Wakeup();

// The slave will immediately enter operational state:
assert(slave->Status() == LinControllerStatus::Operational);

// The master will receive the wake up pulse and the registered callback is
// triggered by the controller:
master_WakeupHandler(LinWakeupEvent{ timestamp, master } );

// The registered callback puts the master back into operational state, i.e.:
assert(master->Status() == LinControllerStatus::Operational);
