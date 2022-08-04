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
// Setup of the Participants
auto participant1 = SilKit::CreateParticipant(config, participantName1, registryUri);
auto participant2 = SilKit::CreateParticipant(config, participantName2, registryUri);

auto* systemMonitor = participant1->CreateSystemMonitor();

// Register ParticipantStatusHandler to receive ParticipantStatus transitions
auto participantStatusHandler =
    [](const ParticipantStatus& participantStatus) {};
systemMonitor->RegisterParticipantStatusHandler(participantStatusHandler);

// Register SystemStateHandler to receive SystemState transitions
auto systemStateHandler =
    [](SystemState state) {};
systemMonitor->RegisterSystemStateHandler(systemStateHandler);

// ------------------------------------------------------------
// Transition from Invalid to ServicesCreated.

// LifecycleService needs to call StartLifecycle for a transition to ParticipantState::ServicesCreated.
// For more information about the use of the life cycle service and time synchronization service refer to the corresponding section.
auto* lifecycleService1 = participant1 -> CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
auto* timeSyncService1 = lifecycleService1 -> CreateTimeSyncService();
auto* lifecycleService2 = participant2 -> CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
auto* timeSyncService2 = lifecycleService2 -> CreateTimeSyncService();

timeSyncService1->SetSimulationStepHandler(
    [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}, 1ms
);
timeSyncService2->SetSimulationStepHandler(
  [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}, 1ms
);

lifecycleService1->StartLifecycle();

// The call of Run() leads to a participant state transition from Invalid to ServicesCreated
// and will trigger the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName1
//  - participantStatus.state == ParticipantState::ServicesCreated
//  - participantStatus.reason = "LifecycleService::StartLifecycle was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

lifecycleService2->StartLifecycle();

// The call of Run() by the second participant again triggers
// the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName2
//  - participantStatus.state == ParticipantState::ServicesCreated
//  - participantStatus.reason = "LifecycleService::StartLifecycle was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

// Since all participants are now in ParticipantState::ServicesCreated,
// the callback of the SystemStateHandler is triggered with SystemState::ServicesCreated:
systemStateHandler(state);
