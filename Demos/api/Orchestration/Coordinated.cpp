// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <thread>

#include "silkit/SilKit.hpp"

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Wrong number of arguments! Start demo with: " << argv[0] << " <ParticipantName>" << std::endl;
        return -1;
    }
    std::string participantName(argv[1]);

    try
    {
        // Setup participant, lifecycle, time synchronization and logging
        const std::string registryUri = "silkit://localhost:8500";
        const std::string configString = R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":"Info"}]}})";
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto logger = participant->GetLogger();

        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

        // Start the worker thread and wait for the go from the system monitor
        auto unleashWorkerThreadPromise = std::promise<void>();
        auto unleashWorkerThreadFuture = unleashWorkerThreadPromise.get_future();
        auto systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler(
            [&unleashWorkerThreadPromise, participantName,
             logger](const SilKit::Services::Orchestration::ParticipantStatus& participantStatus) {
            if (participantStatus.participantName == participantName
                && participantStatus.state == SilKit::Services::Orchestration::ParticipantState::Running)
            {
                logger->Info("The sil-kit-system-controller started the simulation. My state is now 'Running'.");
                unleashWorkerThreadPromise.set_value();
            }
        });

        // Start the worker thread and wait for the go from the system monitor
        std::atomic<bool> workerThreadDone{false};
        auto workerThread = std::thread([&unleashWorkerThreadFuture, &workerThreadDone, logger]() {
            unleashWorkerThreadFuture.wait();
            while (!workerThreadDone)
            {
                std::this_thread::sleep_for(1s);
                logger->Info("Simulation running. Stop via CTRL-C in the sil-kit-system-controller.");
            };

        });

        // Start and wait until the sil-kit-system-controller is stopped
        logger->Info(
            "Start the participant lifecycle and wait for the sil-kit-system-controller to start the simulation.");
        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();

        // Clean up the worker thread
        workerThreadDone = true;

        if (workerThread.joinable())
        {
            workerThread.join();
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -3;
    }

    return 0;
}
