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
        // Setup participant, lifecycle, time synchronization and logging.
        const std::string registryUri = "silkit://localhost:8500";
        const std::string configString = R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":"Info"}]}})";
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto logger = participant->GetLogger();

        // Create an autonomous lifecycle
        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Autonomous});


        // Future / promise to control entrance of the main loop in the worker thread
        std::promise<void> startWorkPromise;
        auto startWorkFuture = startWorkPromise.get_future();

        // The worker thread is 'unleashed' in the starting handler...
        lifecycleService->SetStartingHandler([&startWorkPromise]() { startWorkPromise.set_value(); });

        // Start the worker thread and wait for the go from the starting handler.
        auto workerThread = std::thread([&startWorkFuture, lifecycleService, logger]() {
            startWorkFuture.get();

            for (int i = 0; i < 10; ++i)
            {
                std::this_thread::sleep_for(1s);
                logger->Info("Simulation stop in " + std::to_string(10 - i));
            };

            logger->Info("Stopping just me.");
            lifecycleService->Stop("Stopping just me");
        });

        // Start and wait until lifecycleService->Stop is called.
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
        return -2;
    }

    return 0;
}
