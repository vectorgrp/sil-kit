// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <future>
#include <iostream>
#include <sstream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/netsim/all.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "src/MySimulatedNetwork.hpp"

using namespace std::chrono_literals;
using namespace SilKit::Services;
using namespace SilKit::Services::Can;
using namespace SilKit::Experimental::NetworkSimulation;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl;
        return -1;
    }

    std::string participantConfigurationFilename(argv[1]);
    std::string participantName(argv[2]);
    std::string registryUri = "silkit://localhost:8500";
    if (argc == 4)
        registryUri = argv[3];

    try
    {
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        auto operationMode = SilKit::Services::Orchestration::OperationMode::Coordinated;
        auto* lifecycleService = participant->CreateLifecycleService({operationMode});
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        // ---------------------
        // NETSIM API USAGE
        // ---------------------

        auto scheduler = std::make_unique<Scheduler>();
        auto networkSimulator = SilKit::Experimental::Participant::CreateNetworkSimulator(participant.get());
        std::string networkName = "CAN1";
        SimulatedNetworkType networkType = SimulatedNetworkType::CAN;
        auto simulatedCanNetwork =
            std::make_unique<MySimulatedNetwork>(participant.get(), scheduler.get(), networkType, networkName);
        networkSimulator->SimulateNetwork(networkName, networkType, std::move(simulatedCanNetwork));

        networkSimulator->Start();

        // ---------------------

        timeSyncService->SetSimulationStepHandler(
            [scheduler = scheduler.get()](std::chrono::nanoseconds now,
                                                                  std::chrono::nanoseconds /*duration*/) {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                scheduler->OnSimulationStep(now);
                std::this_thread::sleep_for(1s);
            },
            1ms);

        auto lifecycleFuture = lifecycleService->StartLifecycle();
        auto finalState = lifecycleFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        
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
