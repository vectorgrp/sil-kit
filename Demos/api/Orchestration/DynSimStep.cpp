// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <random>
#include <thread>

#include "silkit/SilKit.hpp"

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    out << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << "ms";
    return out;
}

int main(int argc, char** argv)
{
    if (argc != 5 && argc != 6)
    {
        std::cerr << "Wrong number of arguments! Start demo with: " << argv[0] << 
            " <ParticipantName> "
            "<StepSize> "
            "[-A (Autonomous) | -C (Coordinated)] "
            "[-M (ByMinimalDuration) | -D (ByOwnDuration)] "
            "[-R (Optional; Randomize StepSize (1ms to 10ms) with 10% probability every step)]" << std::endl;
        return -1;
    }
    // Arg 1: Participant Name
    std::string participantName(argv[1]);

    // Arg 2: Step Size
    auto stepSize = std::chrono::milliseconds(std::stoi(argv[2]));
    std::cout << "Starting with stepSize=" << stepSize << std::endl;

    // Arg 3: Operation Mode
    auto operationMode = SilKit::Services::Orchestration::OperationMode::Coordinated;
    if (std::string(argv[3]) == "-A")
    {
        std::cout << "Using OperationMode::Autonomous" << std::endl;
        operationMode = SilKit::Services::Orchestration::OperationMode::Autonomous;
    }
    else if (std::string(argv[3]) == "-C")
    {
        std::cout << "Using OperationMode::Coordinated" << std::endl;
    }
    else
    {
        std::cerr << "Unknown third argument '" << argv[3] << "'. Did you mean '-A' for autonomous mode or '-C' for coordinated mode?" << std::endl;
        return -1;
    }

    // Arg 4: Time Advance Mode
    auto timeAdvanceMode = SilKit::Services::Orchestration::TimeAdvanceMode::ByMinimalDuration;
    if (std::string(argv[4]) == "-M")
    {
        std::cout << "Using TimeAdvanceMode::ByMinimalDuration" << std::endl;
    }
    else if (std::string(argv[4]) == "-D")
    {
        timeAdvanceMode = SilKit::Services::Orchestration::TimeAdvanceMode::ByOwnDuration;
        std::cout << "Using TimeAdvanceMode::ByOwnDuration" << std::endl;
    }
    else
    {
        std::cerr << "Unknown argument '" << argv[4]
                  << "'. Did you mean '-M' for TimeAdvanceMode::ByMinimalDuration or '-D' for TimeAdvanceMode::ByOwnDuration?" << std::endl;
        return -1;
    }

    // Arg 5: Optional Randomize Step Size
    bool randomizeStepSize = false;
    if (argc == 6)
    {
        if (std::string(argv[5]) == "-R")
        {
            randomizeStepSize = true;
            std::cout << "Randomizing step size every 10 steps." << std::endl << std::endl;
        }
        else
        {
            std::cerr << "Unknown argument '" << argv[5] << "'. Did you mean '-R' to randomize the step size every 10 steps?" << std::endl;
            return -1;
        }
    }

    try
    {
        // Setup participant, lifecycle, time synchronization and logging.
        const std::string registryUri = "silkit://localhost:8500";
        const std::string configString = R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":"Info"}]}})";
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto logger = participant->GetLogger();

        auto* lifecycleService = participant->CreateLifecycleService({operationMode});

        auto* timeSyncService = lifecycleService->CreateTimeSyncService(timeAdvanceMode);

        std::random_device rd;
        std::mt19937 rng(rd());
        auto bounded_rand = [&rng](unsigned range) {
            std::uniform_int_distribution<unsigned> dist(1, range);
            return dist(rng);
        };

        timeSyncService->SetSimulationStepHandler(
            [randomizeStepSize, logger, timeSyncService, participantName, bounded_rand](
                std::chrono::nanoseconds now,
                                                                     std::chrono::nanoseconds duration) {
            // The invocation of this handler marks the beginning of a simulation step.
            {
                std::stringstream ss;
                ss << "--------- Simulation step T=" << now << ", duration=" << duration << " ---------";
                logger->Info(ss.str());
            }

            if (randomizeStepSize && bounded_rand(10) == 1)
            {
                auto rndStepDuration = bounded_rand(10);
                timeSyncService->SetStepDuration(std::chrono::milliseconds(rndStepDuration));
                std::stringstream ss;
                ss << "--------- Changing step size to " << rndStepDuration << "ms ---------";
                logger->Info(ss.str());
            }

            std::this_thread::sleep_for(500ms);

        }, stepSize);

        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -2;
    }

    return 0;
}
