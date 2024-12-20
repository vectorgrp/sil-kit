// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <thread>

#include "silkit/SilKit.hpp"

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    out << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << "ms";
    return out;
}

std::mutex mx;
bool doStep = false;
std::condition_variable cv;
std::atomic<bool> asyncThreadDone{false};

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

        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        // 1. The simulation step gets called by the SIL Kit Time Synchronization algorithm.
        // 2. The simulation step handler unlocks a step in the asyncThread with a condition variable.
        // 3. The asyncThread performs some asynchronous operation.
        // 4. The asyncThread completes the simulation step.

        // Simulation steps
        const auto stepSize = 1ms;
        timeSyncService->SetSimulationStepHandlerAsync(
            [logger](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            std::stringstream ss;
            ss << "--------- Simulation step T=" << now << " ---------";
            logger->Info(ss.str());

            std::unique_lock<decltype(mx)> lock(mx);
            doStep = true;
            cv.notify_one();

        }, stepSize);

        auto asyncThread = std::thread([timeSyncService, logger]() {
            while (!asyncThreadDone)
            {
                std::unique_lock<decltype(mx)> lock(mx);
                cv.wait(lock, [] { return doStep; });
                doStep = false;

                logger->Info("Asynchronous operation in the simulation step:");
                logger->Info("  Sending a message to another participant...");
                std::this_thread::sleep_for(0.5s);
                logger->Info("  Awaiting the reply of another participant...");
                std::this_thread::sleep_for(0.5s);
                logger->Info("  Calling a REST API...");
                std::this_thread::sleep_for(0.5s);
                logger->Info("Done.");

                timeSyncService->CompleteSimulationStep();
            }
        });

        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();

        asyncThreadDone = true;
        if (asyncThread.joinable())
        {
            asyncThread.join();
        }

    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -3;
    }

    return 0;
}
