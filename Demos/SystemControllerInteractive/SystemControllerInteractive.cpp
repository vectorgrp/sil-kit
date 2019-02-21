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
#include "ib/cfg/string_utils.hpp"


using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;

using namespace std::chrono_literals;

//std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
//{
//    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
//    out << seconds.count() << "s";
//    return out;
//}

sync::SystemCommand::Kind ToSystemCommand(const std::string& cmdString)
{
    if (cmdString == "Invalid")
        return SystemCommand::Kind::Invalid;
    else if (cmdString == "Run")
        return SystemCommand::Kind::Run;
    else if (cmdString == "Stop")
        return SystemCommand::Kind::Stop;
    else if (cmdString == "Shutdown")
        return SystemCommand::Kind::Shutdown;
    else if (cmdString == "PrepareColdswap")
        return SystemCommand::Kind::PrepareColdswap;
    else if (cmdString == "ExecuteColdswap")
        return SystemCommand::Kind::ExecuteColdswap;

    throw ib::type_conversion_error{};
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

sync::ParticipantCommand::Kind ToParticipantCommand(const std::string& cmdString)
{
    if (cmdString == "Invalid")
        return ParticipantCommand::Kind::Invalid;
    else if (cmdString == "Initialize")
        return ParticipantCommand::Kind::Initialize;
    else if (cmdString == "ReInitialize")
        return ParticipantCommand::Kind::ReInitialize;

    throw ib::type_conversion_error{};
}

class InteractiveSystemController
{
public:
    InteractiveSystemController(ISystemController* controller, cfg::Config ibConfig, std::string myName)
        : _config{std::move(ibConfig)}
        , _myParticipantName{std::move(myName)}
        , _controller{controller}
    {
        ReportSystemConfiguration();
    }

    void ReportSystemConfiguration()
    {
        std::cout << "The following participants are configured:\n";
        for (auto&& participantCfg : _config.simulationSetup.participants)
        {
            std::cout << participantCfg.id << ":\t" << participantCfg.name << " ";
            if (participantCfg.name == _myParticipantName)
                std::cout << "(this process)\t";

            std::cout << "SyncType: " << participantCfg.syncType << std::endl;
        }
    }

    void RunInteractiveLoop()
    {
        while (true)
        {
            ParseAndDispatchCommand(std::cin);
        }
    }

    void ParseAndDispatchCommand(std::istream& instream)
    {
        std::string cmdRraw;
        while (std::getline(instream, cmdRraw))
        {
            std::istringstream cmdStream(cmdRraw);

            std::string cmdName;
            cmdStream >> cmdName;

            try
            {
                SystemCommand::Kind systemCommand = ToSystemCommand(cmdName);
                SendCommand(systemCommand);
                continue;
            }
            catch (const ib::type_conversion_error&)
            {
                // pass
            }

            try
            {
                ParticipantCommand::Kind participantCommand = ToParticipantCommand(cmdName);
                mw::ParticipantId participantId;
                cmdStream >> participantId;

                SendCommand(participantCommand, participantId);
                continue;
            }
            catch (const ib::type_conversion_error&)
            {
                // pass
            }

            std::cerr << "Invalid command: \"" << cmdName << "\"" << std::endl;
        }

    }

    void SendCommand(SystemCommand::Kind systemCommand)
    {
        std::cout << "Sending: SystemCommand::" << systemCommand << std::endl;
        switch (systemCommand)
        {
        case SystemCommand::Kind::Invalid:
            return;
        case SystemCommand::Kind::Run:
            _controller->Run();
            return;
        case SystemCommand::Kind::Stop:
            _controller->Stop();
            return;
        case SystemCommand::Kind::Shutdown:
            _controller->Shutdown();
            return;
        case SystemCommand::Kind::PrepareColdswap:
            _controller->PrepareColdswap();
            return;
        case SystemCommand::Kind::ExecuteColdswap:
            _controller->ExecuteColdswap();
            return;
        }
    }

    void SendCommand(ParticipantCommand::Kind participantCommand, mw::ParticipantId participantId)
    {
        std::cout << "Sending: ParticipantCommand::" << participantCommand << " to " << participantId << std::endl;
        switch (participantCommand)
        {
        case ParticipantCommand::Kind::Invalid:
            return;
        case ParticipantCommand::Kind::Initialize:
            _controller->Initialize(participantId);
            return;
        case ParticipantCommand::Kind::ReInitialize:
            _controller->ReInitialize(participantId);
        }
    }

private:
    cfg::Config _config;
    std::string _myParticipantName;
    ISystemController* _controller{nullptr};
};

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
    std::string participantName{"SystemController"};

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

    std::cout << "Creating interactive SystemController for IB domain=" << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);

    auto systemMonitor = comAdapter->GetSystemMonitor();
    systemMonitor->RegisterParticipantStatusHandler(&ReportParticipantStatus);
    systemMonitor->RegisterSystemStateHandler(&ReportSystemState);

    InteractiveSystemController systemController(comAdapter->GetSystemController(), ibConfig, participantName);
    systemController.RunInteractiveLoop();

    return 0;
}
