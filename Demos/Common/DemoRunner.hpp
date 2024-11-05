#pragma once

#include <future>
#include <iostream>
#include <sstream>
#include <thread>

#include "silkit/Silkit.hpp"
#include "IDemo.hpp"
#include "DemoCommandLineParser.hpp"

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

using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::Logging;

namespace SilKitDemo {

int Run(std::string defaultParticipantName, IDemo& demo, int argc, char** argv)
{
    try
    {
        const auto stepSize = 1ms;

        RunArgs runArgs{};
        auto returnCode = SilKitDemo::ParseCommandLineArguments2(argc, argv, defaultParticipantName, runArgs);
        if (returnCode <= 0)
        {
            return returnCode;
        }

        auto participant =
            SilKit::CreateParticipant(runArgs.participantConfiguration, runArgs.participantName, runArgs.registryUri);
        auto* logger = participant->GetLogger();
        auto operationMode = (runArgs.runSync ? OperationMode::Coordinated : OperationMode::Autonomous);
        auto* lifecycleService = participant->CreateLifecycleService({operationMode});

        Context context{participant.get(), lifecycleService, logger};
        demo.CreateControllers(context);

        lifecycleService->SetStopHandler([logger]() { logger->Info("Stopped"); });
        lifecycleService->SetShutdownHandler([logger]() { logger->Info("Shutdown"); });
        lifecycleService->SetAbortHandler([logger](auto lastState) { logger->Warn("Aborted"); });
        lifecycleService->SetCommunicationReadyHandler([runArgs, &context, logger, &demo]() {
            std::cout << runArgs.participantName << " is ready for communication" << std::endl;
            demo.InitControllers(context);
        });

        if (runArgs.runSync)
        {
            auto* timeSyncService = lifecycleService->CreateTimeSyncService();
            timeSyncService->SetSimulationStepHandler(
                [&demo, &context](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                demo.DoWork(context);
            }, stepSize);


            auto finalStateFuture = lifecycleService->StartLifecycle();
            finalStateFuture.get();
        }
        else
        {
            std::thread workerThread;
            std::promise<void> startMainLoopPromise;
            std::future<void> startMainLoopFuture = startMainLoopPromise.get_future();
            workerThread = std::thread{[&context, &startMainLoopFuture, lifecycleService, &demo]() {
                startMainLoopFuture.get();
                while (lifecycleService->State() == ParticipantState::ReadyToRun
                       || lifecycleService->State() == ParticipantState::Running)
                {
                    demo.DoWork(context);
                }
            }};
            lifecycleService->SetStartingHandler([&]() { startMainLoopPromise.set_value(); });

            auto finalStateFuture = lifecycleService->StartLifecycle();
            std::cout << "Press enter to leave the simulation..." << std::endl;
            std::cin.ignore();

            if (lifecycleService->State() == ParticipantState::Running
                || lifecycleService->State() == ParticipantState::Paused)
            {
                lifecycleService->Stop("User requested to stop");
            }

            finalStateFuture.get();

            if (workerThread.joinable())
            {
                workerThread.join();
            }
        }

        demo.Teardown(context);

        return 0;
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }
}

} // namespace SilKitDemo