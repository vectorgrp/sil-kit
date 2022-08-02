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

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/can/all.hpp"
#include "silkit/services/can/string_utils.hpp"


using namespace SilKit::Services;
using namespace SilKit::Services::Can;

using namespace std::chrono_literals;

namespace std {
namespace chrono {
std::ostream& operator<<(std::ostream& out, nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}
} // namespace chrono
} // namespace std

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri] [--uncoordinated] "
                     "[--async]"
                  << std::endl;
        return -1;
    }

    if (argc > 6)
    {
        std::cerr << "Too many arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri] [--uncoordinated] "
                     "[--async]"
                  << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        std::string registryUri = "silkit://localhost:8500";

        SilKit::Services::Orchestration::OperationMode coordinateStartAndStop =
            SilKit::Services::Orchestration::OperationMode::Coordinated;
        bool runSync = true;

        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (const auto& arg : args)
        {
            if (arg == "--autonomous")
            {
                coordinateStartAndStop = SilKit::Services::Orchestration::OperationMode::Autonomous;
            }
            else if (arg == "--async")
            {
                runSync = false;
            }
            else
            {
                registryUri = arg;
            }
        }

        // only used for async setups to stop the participant
        std::thread workerThread;

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);
        auto sleepTimePerTick = 1000ms;

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        if (runSync)
        {
            // In this demo, the life cycle service will always be used
            auto* lifecycleService = participant->CreateLifecycleService();

            // Set a CommunicationReady Handler
            lifecycleService->SetCommunicationReadyHandler([]() {
                std::cout << "CommunicationReady..." << std::endl;
            });

            // Set a Stop Handler
            lifecycleService->SetStopHandler([]() {
                std::cout << "Stopping..." << std::endl;
            });

            // Set a Shutdown Handler
            lifecycleService->SetShutdownHandler([]() {
                std::cout << "Shutdown..." << std::endl;
            });

            // configure time synchronization
            auto timeSyncService = lifecycleService->CreateTimeSyncService();

            timeSyncService->SetSimulationStepHandler(
                [&, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                    std::cout << "now=" << now << ", duration=" << duration << std::endl;
                    std::this_thread::sleep_for(sleepTimePerTick);

                    if (participantName == "PauseTest")
                    {
                        if (now == 20ms || now == 30ms)
                        {
                            std::cout << "Pausing..." << std::endl;
                            auto pauseTime = std::chrono::system_clock::now();
                            lifecycleService->Pause("Manual pause");
                            std::this_thread::sleep_for(3s);
                            lifecycleService->Continue();

                            auto diffTimeInSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::system_clock::now() - pauseTime);

                            std::cout << "Continuing after " << diffTimeInSeconds.count() << "s..." << std::endl;
                        }
                    }
                }, 5ms);
            auto finalStateFuture = lifecycleService->StartLifecycle(
                Orchestration::LifecycleConfiguration{coordinateStartAndStop});
            auto finalState = finalStateFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            // In this demo, the life cycle service will always be used
            auto* lifecycleService = participant->CreateLifecycleService();

            // Set a CommunicationReady Handler
            lifecycleService->SetCommunicationReadyHandler([]() {
                std::cout << "CommunicationReady..." << std::endl;
            });

            // Set a Starting Handler (only triggers for asynchronous participants)
            lifecycleService->SetStartingHandler([&]() {
                std::cout << "Starting..." << std::endl;
                workerThread = std::thread{[&]() {
                    std::this_thread::sleep_for(2s);
                    std::cout << "Stopping participants after 2s manually..." << std::endl;
                    lifecycleService->Stop("Manual stop.");
                }};
            });

            // Set a Stop Handler
            lifecycleService->SetStopHandler([]() {
                std::cout << "Stopping..." << std::endl;
            });

            // Set a Shutdown Handler
            lifecycleService->SetShutdownHandler([]() {
                std::cout << "Shutdown..." << std::endl;
            });

            auto finalStateFuture = lifecycleService->StartLifecycle(
                Orchestration::LifecycleConfiguration{coordinateStartAndStop});
            auto finalState = finalStateFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
            workerThread.join();
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
