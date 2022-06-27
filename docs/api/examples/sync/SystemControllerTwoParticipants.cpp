// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Setup of the Participants
auto participant1 = ib::CreateParticipant(ibConfig, participantName1, domainId, true);
auto participant2 = ib::CreateParticipant(ibConfig, participantName2, domainId, true);

auto* systemController = participant1->GetSystemController();
auto* systemMonitor = participant1->GetSystemMonitor();

// Tell the SystemController to expect the two synchronized participants
systemController->SetWorkflowConfiguration({participantName1, participantName2});

// Register SystemStateHandler to trigger the commands of the System Controller in the correct system states.
// For more information about the use of the System Monitor refer to the corresponding section.
auto systemStateHandler = 
    [systemController, ibConfig](SystemState state) {
        switch (state)
        {
        case SystemState::ServicesCreated:
            // ------------------------------------------------------------
            // Transition from SystemState::ServicesCreated to SystemState::ReadyToRun:
            systemController->Initialize(participantName1);
            systemController->Initialize(participantName2);

            return;
        case SystemState::ReadyToRun:
            // ------------------------------------------------------------
            // Transition from SystemState::ReadyToRun to SystemState::Running:
            systemController->Run();
            return;
        case SystemState::Stopped:
            // ------------------------------------------------------------
            // Transition from SystemState::Stopped to SystemState::Shutdown:
            systemController->Shutdown();
            return;
        }
};

systemMonitor->RegisterSystemStateHandler(systemStateHandler);

// ------------------------------------------------------------
// Start of the simulation.

// LifecycleService needs to call ExecuteLifecycleWithSyncTime or ExecuteLifecycleNoSyncTime for a transition to ParticipantState::ServicesCreated.
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

auto status1 = lifecycleService1 -> ExecuteLifecycleWithSyncTime(timeSyncService1, true, true);
auto status2 = lifecycleService2 -> ExecuteLifecycleWithSyncTime(timeSyncService2, true, true);

// As soon as all participants are in ParticipantState::ServicesCreated, the system transitions to SystemState::ServicesCreated
// and the systemController calls Run() to start the simulation.
std::this_thread::sleep_for(5s); //give the system some time to run
// ------------------------------------------------------------
// Stopping the simulation (Transition from SystemState::Running to SystemState::Stopped).
systemController->Stop();

// As soon as all participants did stop the simulation and the system is in SystemState::Stopped,
// the systemController calls the final Shutdown() command.
