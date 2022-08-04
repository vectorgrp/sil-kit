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

auto* systemController = participant1->CreateSystemController();
auto* systemMonitor = participant1->CreateSystemMonitor();

// Tell the SystemController to expect the two synchronized participants
systemController->SetWorkflowConfiguration({participantName1, participantName2});

// Register SystemStateHandler to trigger the commands of the System Controller in the correct system states.
// For more information about the use of the System Monitor refer to the corresponding section.
auto systemStateHandler = 
    [systemController, config](SystemState state) {
        switch (state)
        {
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

// LifecycleService needs to call StartLifecycle for a transition to ParticipantState::ServicesCreated.
// For more information about the use of the life cycle service and time synchronization service refer to the corresponding section.
auto* lifecycleService1 = participant1 -> CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
auto* timeSyncService1 = lifecycleService1 -> CreateTimeSynchrService();
auto* lifecycleService2 = participant2 -> CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
auto* timeSyncService2 = lifecycleService2 -> CreateTimeSynchrService();

timeSyncService1->SetSimulationStepHandler(
    [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}, 1ms
);
timeSyncService2->SetSimulationStepHandler(
  [](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {}, 1ms
);

auto status1 = lifecycleService1 -> StartLifecycle();
auto status2 = lifecycleService2 -> StartLifecycle();

// As soon as all participants are in ParticipantState::ServicesCreated, the system transitions to SystemState::ServicesCreated
// and the SystemController calls Run() to start the simulation.
std::this_thread::sleep_for(5s); //give the system some time to run
// ------------------------------------------------------------
// Stopping the simulation (Transition from SystemState::Running to SystemState::Stopping).
lifecycleService1->Stop("Stop");

// All participants will traverse the ParticipantState::Stopped, ParticipantState::ShuttingDown and ParticipantState::Shutdown states. 
// Afterwards, status1/status2 will return.
