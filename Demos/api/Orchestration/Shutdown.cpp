// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//

#include <iostream>
#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <memory>
#include <functional>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

using namespace std::chrono_literals;
using namespace SilKit::Services::Orchestration;

std::unique_ptr<std::thread> timeoutThread;
std::atomic_bool participantDeleted{false};
std::atomic_bool timerActive{false};
std::atomic_bool startWorkPromiseSet{false};
std::unique_ptr<SilKit::IParticipant> participant;

void OnTimerTimeout()
{
    std::cout << "Timeout reached, destroying participant (nongraceful)." << std::endl;
    timerActive = false;
    if (participant)
    {
        participantDeleted = true;
        participant.reset();
    }
}

void StartTimeoutTimer(std::chrono::seconds timeout)
{
    timerActive = true;
    timeoutThread = std::make_unique<std::thread>([timeout]() {
        for (auto remaining = timeout.count(); remaining > 0; --remaining)
        {
            if (!timerActive)
                return;
            std::cout << "Nongraceful stop in " << remaining << " seconds..." << std::endl;
            std::this_thread::sleep_for(1s);
        }
        if (timerActive)
        {
            OnTimerTimeout();
        }
    });
}

std::atomic_bool stopRequested{false};	

// Try to stop if running, otherwise request a stop and start a timer
void TryStop(ILifecycleService* lifecycleService)
{
    if (stopRequested)
    {
        return;
    }

    stopRequested = true;

    auto state = lifecycleService->State();

    if (state == ParticipantState::Running || state == ParticipantState::Paused)
    {
        lifecycleService->Stop("Stop while running/paused."); // graceful
    }
    else
    {
        StartTimeoutTimer(std::chrono::seconds{5});
    }
}

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

        participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto logger = participant->GetLogger();

        // Create a coordinated lifecycle
        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

         // Future / promise to control entrance of the main loop in the worker thread
        std::promise<void> startWorkPromise;
        std::future<void> startWorkFuture;
        startWorkFuture = startWorkPromise.get_future();


		// Wait for the "Running" state on this participant and handle the requested stop
        auto* systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler(
            [lifecycleService, participantName, logger](const ParticipantStatus& status) {

            if (participantName != status.participantName)
                return;

            std::stringstream ss;
            ss << "Participant state: " << status.state;
            logger->Info(ss.str());

            if (status.state == ParticipantState::ServicesCreated)
            {
                logger->Info("TryStop in ServicesCreated");
                //TryStop(lifecycleService);
            }

            if (stopRequested && (status.state == ParticipantState::Running || status.state == ParticipantState::Paused))
            {
                lifecycleService->Stop("Requested stop."); // graceful
            }
        });

        // The worker thread is 'unleashed' in the starting handler...
        lifecycleService->SetStartingHandler([&startWorkPromise]() { 
            startWorkPromiseSet = true;
            startWorkPromise.set_value(); 
            });

        // Start the worker thread and wait for the go from the starting handler.
        std::atomic<bool> workerThreadDone{false};
        auto workerThread =
            std::thread([&startWorkFuture, &workerThreadDone, logger]() {

            startWorkFuture.get();
            while (!workerThreadDone)
            {
                std::this_thread::sleep_for(1s);
                if (!participantDeleted)
                    logger->Info("Simulation running.");
            };
        });

        // Start and wait until the sil-kit-system-controller is stopped.
        logger->Info(
            "Start the participant lifecycle and wait for the sil-kit-system-controller to start the simulation.");
        auto finalStateFuture = lifecycleService->StartLifecycle();

        try
        {
            finalStateFuture.get();
        }
        catch (const std::exception& /*e*/)
        {
            std::cout << "Participant already destroyed" << std::endl;
        }

        if (participant)
        {
            participantDeleted = true;
            participant.reset();
        }

        // Clean up the worker/timeout thread.
        workerThreadDone = true;
        if (!startWorkPromiseSet)
        {
            startWorkPromise.set_value(); 
        }

        if (workerThread.joinable())
        {
            workerThread.join();
        }

        if (timeoutThread && timeoutThread.get()->joinable())
        {
            timeoutThread.get()->join();
        }
        
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -2;
    }

    return 0;
}
