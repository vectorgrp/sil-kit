// Copyright (c)  Vector Informatik GmbH. All rights reserved.

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
    std::time_t enterTime = std::chrono::system_clock::to_time_t(status.enterTime);
    std::tm tmBuffer;
#if defined(_MSC_VER)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif

    char timeString[32];
    std::strftime(timeString, sizeof(timeString), "%F %T", &tmBuffer);

    std::cout
        << timeString
        << " " << status.participantName
        << "\t State: " << status.state
        << "\t Reason: " << status.enterReason
        << std::endl;
}

void ReportSystemState(ib::mw::sync::SystemState state)
{
    std::cout << "New SystemState: " << state << std::endl;
}

int main(int argc, char** argv)
{
    ib::cfg::Config ibConfig;
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        auto jsonFilename = std::string(argv[1]);
        ibConfig = ib::cfg::Config::FromJsonFile(jsonFilename);
    }
    catch (ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << (&error)->what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    std::string participantName{"SystemMonitor"};

    uint32_t domainId = 42;
    if (argc >= 3)
    {
        try
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[2]));
        }
        catch (std::exception&)
        {
        }
    }

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    try
    {
        auto comAdapter = ib::CreateFastRtpsComAdapter(std::move(ibConfig), participantName, domainId);

        auto systemMonitor = comAdapter->GetSystemMonitor();
        systemMonitor->RegisterParticipantStatusHandler(&ReportParticipantStatus);
        systemMonitor->RegisterSystemStateHandler(&ReportSystemState);

        std::cout << "Press enter to terminate the SystemMonitor..." << std::endl;
        std::cin.ignore();
    }
    catch (const std::exception& e)
    {
        std::cout << "Something went wrong: " << e.what() << std::endl;
        std::cout << "Press enter to terminate the SystemMonitor..." << std::endl;
        std::cin.ignore();
    }

    return 0;
}
