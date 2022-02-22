// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS 1

#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/mw/logging/ILogger.hpp"

using namespace ib::mw;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

int main(int argc, char** argv) try
{
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> [domainId]" << std::endl;
        return -1;
    }

    std::string participantConfigurationFilename(argv[1]);
    std::string participantName{"SystemMonitor"};

    uint32_t domainId = 42;
    if (argc >= 3)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[2]));
    }

    auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto comAdapter = ib::CreateSimulationParticipant(std::move(participantConfiguration), participantName, domainId, false);
    auto* logger = comAdapter->GetLogger();
    auto* systemMonitor = comAdapter->GetSystemMonitor();
    
    systemMonitor->RegisterParticipantStatusHandler(
        [logger](const sync::ParticipantStatus& status) {
            std::stringstream buffer;
            buffer
                << "New ParticipantState: " << status.participantName << " is " << status.state
                << ",\tReason: " << status.enterReason;
            logger->Info(buffer.str());
        });
    
    systemMonitor->RegisterSystemStateHandler(
        [logger](sync::SystemState state) {
            std::stringstream buffer;
            buffer << "New SystemState: " << state;
            logger->Info(buffer.str());
        });

    std::cout << "Press enter to terminate the SystemMonitor..." << std::endl;
    std::cin.ignore();
    return 0;
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
