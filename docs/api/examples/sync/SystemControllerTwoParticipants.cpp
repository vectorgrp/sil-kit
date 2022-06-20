// Copyright (c) Vector Informatik GmbH. All rights reserved.
// ------------------------------------------------------------
// Setup of the Participants
auto participant1 = ib::CreateParticipant(ibConfig, participantName1, domainId, true);
auto participant2 = ib::CreateParticipant(ibConfig, participantName2, domainId, true);

auto* systemController = participant1->GetSystemController();
auto* systemMonitor = participant1->GetSystemMonitor();

// Tell the SystemController to expect the two synchronized participants
systemController->SetRequiredParticipants({participantName1, participantName2});

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

// ParticipantController needs to call Run or RunAsync for a transition to ParticipantState::ServicesCreated.
// For more information about the use of the Participant Controller refer to the corresponding section.
auto* participantController1 = participant1->GetParticipantController();
auto* participantController2 = participant2->GetParticipantController();

participantController1->SetSimulationTask(
    [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);
participantController2->SetSimulationTask(
  [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);

auto status1 = participantController1->RunAsync();
auto status2 = participantController2->RunAsync();

// As soon as all participants are in ParticipantState::ServicesCreated, the system transitions to SystemState::ServicesCreated
// and the systemController calls Run() to start the simulation.
std::this_thread::sleep_for(5s); //give the system some time to run
// ------------------------------------------------------------
// Stopping the simulation (Transition from SystemState::Running to SystemState::Stopped).
systemController->Stop();

// As soon as all participants did stop the simulation and the system is in SystemState::Stopped,
// the systemController calls the final Shutdown() command.
