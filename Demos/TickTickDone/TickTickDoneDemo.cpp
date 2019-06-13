// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstring>

#include <sstream>
#include <iostream>
#include <string>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/all.hpp"

using namespace ib::mw;
using namespace ib::sim;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void SystemStateHandler(sync::ISystemController* controller, sync::SystemState newState, const ib::cfg::Config& ibConfig)
{
    switch (newState)
    {
    case ib::mw::sync::SystemState::Idle:
    {
        for (auto&& participant : ibConfig.simulationSetup.participants)
        {
            if (participant.name == "master")
                continue;
            controller->Initialize(participant.id);
        }
        break;
    }
    
    case ib::mw::sync::SystemState::Initialized:
        controller->Run();
        break;
    
    case ib::mw::sync::SystemState::Stopped:
        controller->Shutdown();
        break;
    
    default:
        std::cout << "New SystemState " << to_string(newState) << std::endl;
    }
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <NumberOfClients> master|client-<clientId> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        auto numClients = std::stoul(argv[1]);
        auto participantName = std::string(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }

        // Build TTD Configuration using Config Builder Pattern
        ib::cfg::ConfigBuilder configBuilder{"TTD-Demo"};
        auto&& simulationSetup = configBuilder.SimulationSetup();

        // Set TTD Time Synchronization
        simulationSetup
            .ConfigureTimeSync().WithTickPeriod(1ms);

        // add "master" as time sync master
        simulationSetup
            .AddParticipant("master")
            .AsSyncMaster();

        // add "client-0" .. "client-<n-1>" as clients
        for (unsigned long i = 0; i < numClients; i++)
        {
            std::stringstream clientName;
            clientName << "client-" << i;

            simulationSetup
                .AddParticipant(clientName.str())
                .WithSyncType(ib::cfg::SyncType::DiscreteTime);
        }

        auto ibConfig = configBuilder.Build();

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);

        if (participantName == "master")
        {
            auto controller = comAdapter->GetSystemController();
            auto monitor = comAdapter->GetSystemMonitor();

            monitor->RegisterSystemStateHandler([controller, &ibConfig](sync::SystemState newState) {

                SystemStateHandler(controller, newState, ibConfig);

            });

            std::cout << "Press enter to stop and shutdown!" << std::endl;
            std::cin.ignore();
            controller->Stop();
            std::this_thread::sleep_for(1s);
            std::cout << "exiting..." << std::endl;

        }
        else
        {
            auto controller = comAdapter->GetParticipantController();
            controller->SetSimulationTask([](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

                std::cout << "now=" << now << ", duration=" << duration << std::endl;
                std::this_thread::sleep_for(100ms);

            });
            auto finalStateFuture = controller->RunAsync();
            finalStateFuture.wait();
        }
    }
    catch (const ib::cfg::Misconfiguration& error)
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
