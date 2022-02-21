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
    InteractiveSystemController(ISystemController* controller,
                                std::shared_ptr<ib::cfg::IParticipantConfiguration> ibConfig, std::string myName,
                                const std::vector<std::string>& expectedParticipantNames)
        : _ibConfig{std::move(ibConfig)}
        , _expectedParticipantNames{expectedParticipantNames}
        , _myParticipantName{std::move(myName)}
        , _controller{controller}
    {
        ReportSystemConfiguration();
    }

    void ReportSystemConfiguration()
    {
        std::cout << "The following participants are expected:\n";
        for (auto&& name : _expectedParticipantNames)
        {
            std::cout << name << " ";
            if (name == _myParticipantName)
                std::cout << "(this process)\t";
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
                std::string participantName;
                cmdStream >> participantName;

                SendCommand(participantCommand, participantName);
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

    void SendCommand(ParticipantCommand::Kind participantCommand, std::string participantName)
    {
        std::cout << "Sending: ParticipantCommand::" << participantCommand << " to " << participantName << std::endl;
        switch (participantCommand)
        {
        case ParticipantCommand::Kind::Invalid:
            return;
        case ParticipantCommand::Kind::Initialize:
            _controller->Initialize(participantName);
            return;
        case ParticipantCommand::Kind::ReInitialize:
            _controller->ReInitialize(participantName);
        }
    }

private:
    std::shared_ptr<ib::cfg::IParticipantConfiguration> _ibConfig;
    std::vector<std::string> _expectedParticipantNames;
    std::string _myParticipantName;
    ISystemController* _controller{nullptr};
};

int main(int argc, char** argv)
{
    std::shared_ptr<ib::cfg::IParticipantConfiguration> ibConfig;
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> <domainId> [participantName1] [participantName2] ..." << std::endl;
        return -1;
    }
    
    try
    {
        auto jsonFilename = std::string(argv[1]);
        ibConfig = ib::cfg::ReadParticipantConfigurationFromJsonFile(jsonFilename);
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
    try
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[2]));
    }
    catch (std::exception&)
    {
    }

    std::vector<std::string> expectedParticipantNames;
    for (int i = 3; i < argc; i++)
    {
        expectedParticipantNames.push_back(argv[i]);
    }

    std::cout << "Creating interactive SystemController for IB domain=" << domainId << std::endl;
    auto comAdapter = ib::CreateSimulationParticipant(ibConfig, participantName, domainId, false);

    auto systemMonitor = comAdapter->GetSystemMonitor();
    auto systemController = comAdapter->GetSystemController();
    systemController->SetSynchronizedParticipants(expectedParticipantNames);
    systemMonitor->RegisterParticipantStatusHandler(&ReportParticipantStatus);
    systemMonitor->RegisterSystemStateHandler(&ReportSystemState);

    InteractiveSystemController systemControllerIA(comAdapter->GetSystemController(), ibConfig, participantName, expectedParticipantNames);
    systemControllerIA.RunInteractiveLoop();

    return 0;
}
