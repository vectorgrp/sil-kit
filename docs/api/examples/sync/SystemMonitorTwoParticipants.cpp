// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Setup of the Participants
auto comAdapter1 = ib::CreateComAdapter(ibConfig, participantName1, domainId);
auto comAdapter2 = ib::CreateComAdapter(ibConfig, participantName2, domainId);

auto* systemMonitor = comAdapter1->GetSystemMonitor();

// Register ParticipantStatusHandler to receive ParticipantStatus transitions
auto participantStatusHandler =
    [](const ParticipantStatus& participantStatus) {};
systemMonitor->RegisterParticipantStatusHandler(participantStatusHandler);

// Register SystemStateHandler to receive SystemState transitions
auto systemStateHandler =
    [](SystemState state) {};
systemMonitor->RegisterSystemStateHandler(systemStateHandler);


// ------------------------------------------------------------
// Transition from Invalid to Idle.

// ParticipantController needs to call Run or RunAsync for a transition to ParticipantState::Idle.
// For more information about the use of the Participant Controller refer to the corresponding section.
auto* participantController1 = comAdapter1->GetParticipantController();
auto* participantController2 = comAdapter2->GetParticipantController();

participantController1->SetSimulationTask([](now, duration) {});
participantController2->SetSimulationTask([](now, duration) {});

participantController1->Run();

// The call of Run() leads to a participant state transition from Invalid to Idle
// and will trigger the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName1
//  - participantStatus.state == ParticipantState::Idle
//  - participantStatus.reason = "ParticipantController::Run() was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

participantController2->Run();

// The call of Run() by the second participant again triggers
// the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName2
//  - participantStatus.state == ParticipantState::Idle
//  - participantStatus.reason = "ParticipantController::Run() was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

// Since all participants are now in ParticipantState::Idle,
// the callback of the SystemStateHandler is triggered with SystemState::Idle:
systemStateHandler(state);
