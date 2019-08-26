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

#ifdef SendMessage
#undef SendMessage
#endif


using namespace ib::mw;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void ReportParticipantStatus(const ib::mw::sync::ParticipantStatus& status)
{
    // FIXME@fmt:
    //spdlog::info(
    //    "New ParticipantState: {},\tReason: {}",
    //    status.state,
    //    status.enterReason
    //);
}

void ReportSystemState(ib::mw::sync::SystemState state)
{
    // FIXME@fmt: spdlog::info("New SystemState: {}", state);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string jsonFilename(argv[1]);
        std::string participantName{"SystemMonitor"};

        uint32_t domainId = 42;
        if (argc >= 3)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[2]));
        }

        auto ibConfig = ib::cfg::Config::FromJsonFile(jsonFilename);

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(std::move(ibConfig), participantName, domainId);

        auto systemMonitor = comAdapter->GetSystemMonitor();
        systemMonitor->RegisterParticipantStatusHandler(&ReportParticipantStatus);
        systemMonitor->RegisterSystemStateHandler(&ReportSystemState);

        std::cout << "Press enter to terminate the SystemMonitor..." << std::endl;
        std::cin.ignore();
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
