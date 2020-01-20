// ------------------------------------------------------------
// Setup of the Participants
auto comAdapter1 = ib::CreateComAdapter(ibConfig, participantName1, domainId);
auto comAdapter2 = ib::CreateComAdapter(ibConfig, participantName2, domainId);

auto* systemController = comAdapter1->GetSystemController();
auto* systemMonitor = comAdapter1->GetSystemMonitor();

// Register SystemStateHandler to trigger the commands of the System Controller in the correct system states.
// For more information about the use of the System Monitor refer to the corresponding section.
auto systemStateHandler = 
    [systemController, ibConfig](SystemState state) {
        switch (state)
        {
        case SystemState::Idle:
            // ------------------------------------------------------------
            // Transition from SystemState::Idle to SystemState::Initialized:
            for (auto&& participant : ibConfig.simulationSetup.participants)
            {
                // Each participant must be in ParticipantState::Idle.
                systemController->Initialize(participant.id);
            }
            return;
        case SystemState::Initialized:
            // ------------------------------------------------------------
            // Transition from SystemState::Initialized to SystemState::Running:
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

// ParticipantController needs to call Run or RunAsync for a transition to ParticipantState::Idle.
// For more information about the use of the Participant Controller refer to the corresponding section.
auto* participantController1 = comAdapter1->GetParticipantController();
auto* participantController2 = comAdapter2->GetParticipantController();

participantController1->SetSimulationTask(
    [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);
participantController2->SetSimulationTask(
  [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}
);

auto status1 = participantController1->RunAsync();
auto status2 = participantController2->RunAsync();

// As soon as all participants are in ParticipantState::Idle, the system transitions to SystemState::Idle
// and the systemController calls Run() to start the simulation.
std::this_thread::sleep_for(5s); //give the system some time to run
// ------------------------------------------------------------
// Stopping the simulation (Transition from SystemState::Running to SystemState::Stopped).
systemController->Stop();

// As soon as all participants did stop the simulation and the system is in SystemState::Stopped,
// the systemController calls the final Shutdown() command.
