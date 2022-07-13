// ------------------------------------------------------------
// Setup of the Participants
auto participant1 = SilKit::CreateParticipant(config, participantName1, registryUri);
auto participant2 = SilKit::CreateParticipant(config, participantName2, registryUri);

auto* systemMonitor = participant1->GetSystemMonitor();

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

// LifecycleService needs to call StartLifecycleWithSyncTime or StartLifecycleNoSyncTime for a transition to ParticipantState::ServicesCreated.
// For more information about the use of the life cycle service and time synchronization service refer to the corresponding section.
auto* lifecycleService1 = participant1 -> GetLifecycleService();
auto* timeSyncService1 = lifecycleService1 -> GetTimeSynchrService();
auto* lifecycleService2 = participant2 -> GetLifecycleService();
auto* timeSyncService2 = lifecycleService2 -> GetTimeSynchrService();

timeSyncService1->SetSimulationTask(
    [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);
timeSyncService2->SetSimulationTask(
  [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);

lifecycleService1 -> StartLifecycleWithSyncTime(timeSyncService1, true, true);

// The call of Run() leads to a participant state transition from Invalid to ServicesCreated
// and will trigger the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName1
//  - participantStatus.state == ParticipantState::ServicesCreated
//  - participantStatus.reason = "LifecycleService::StartLifecycle... was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

lifecycleService2 -> StartLifecycleWithSyncTime(timeSyncService2, true, true);

// The call of Run() by the second participant again triggers
// the callback of the ParticipantStatusHandler:
participantStatusHandler(participantStatus);
// with:
//  - participantStatus.participantName == participantName2
//  - participantStatus.state == ParticipantState::ServicesCreated
//  - participantStatus.reason = "LifecycleService::StartLifecycle... was called"
//  - participantStatus.enterTime == enter time_point
//  - participantStatus.refreshTime == enter time_point

// Since all participants are now in ParticipantState::ServicesCreated,
// the callback of the SystemStateHandler is triggered with SystemState::ServicesCreated:
systemStateHandler(state);
