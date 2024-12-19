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
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Autonomous});

        // Start the worker thread and wait for the go from the system monitor
        auto unleashWorkerThreadPromise = std::promise<void>();
        auto unleashWorkerThreadFuture = unleashWorkerThreadPromise.get_future();
        auto systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler(
            [&unleashWorkerThreadPromise, participantName,
             logger](const SilKit::Services::Orchestration::ParticipantStatus& participantStatus) {
            if (participantStatus.participantName == participantName && participantStatus.state
                == SilKit::Services::Orchestration::ParticipantState::Running)
            {
                logger->Info("My state is now 'Running'.");
                unleashWorkerThreadPromise.set_value();
            }
        });

        // Start the worker thread and wait for the go from the system monitor
        auto workerThread = std::thread([&unleashWorkerThreadFuture, lifecycleService, logger]() {

            unleashWorkerThreadFuture.wait();
            for (int i = 0; i < 10; ++i)
            {
                std::this_thread::sleep_for(1s);
                logger->Info("Simulation stop in " + std::to_string(10-i));
            };

            logger->Info("Stopping just me.");
            lifecycleService->Stop("Stopping just me");
        });

        // Start and wait until lifecycleService->Stop
        logger->Info("Start the participant lifecycle.");
        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();

        // Clean up the worker thread
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
