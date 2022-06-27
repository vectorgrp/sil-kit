// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS 1

#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>
#include <thread>

#include "ib/version.hpp"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include "CommandlineParser.hpp"

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;

using namespace std::chrono_literals;

sync::SystemCommand::Kind ToSystemCommand(const std::string& cmdString)
{
    if (cmdString == "Invalid")
        return SystemCommand::Kind::Invalid;
    else if (cmdString == "Run")
        return SystemCommand::Kind::Run;
    else if (cmdString == "Stop")
        return SystemCommand::Kind::Stop;

    throw ib::TypeConversionError{};
}

void ReportParticipantStatus(const ib::mw::sync::ParticipantStatus& status)
{
    std::time_t enterTime = std::chrono::system_clock::to_time_t(status.enterTime);
    std::tm tmBuffer;
#if defined(_WIN32)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif

    std::stringstream timeStr;
    timeStr << std::put_time(&tmBuffer, "%F %T ");

    std::cout
        << timeStr.str()
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
    else if (cmdString == "Restart")
        return ParticipantCommand::Kind::Restart;
    else if (cmdString == "Shutdown")
        return ParticipantCommand::Kind::Shutdown;

    throw ib::TypeConversionError{};
}

class InteractiveSystemController
{
public:
    InteractiveSystemController(ISystemController* controller,
                                std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfiguration, std::string myName,
                                const std::vector<std::string>& expectedParticipantNames)
        : _participantConfiguration{std::move(participantConfiguration)}
        , _expectedParticipantNames{expectedParticipantNames}
        , _myParticipantName{std::move(myName)}
        , _controller{controller}
    {
    }

    void RunInteractiveLoop()
    {
        ParseAndDispatchCommand(std::cin);
    }

    void ParseAndDispatchCommand(std::istream& instream)
    {
        std::string cmdRraw;
        std::cout << "> ";
        while (std::getline(instream, cmdRraw))
        {
            std::istringstream cmdStream(cmdRraw);

            std::string cmdName;
            cmdStream >> cmdName;

            if (cmdName == "Exit")
            {
                return;
            }

            try
            {
                SystemCommand::Kind systemCommand = ToSystemCommand(cmdName);
                SendCommand(systemCommand);
                std::cout << "> ";
                continue;
            }
            catch (const ib::TypeConversionError&)
            {
                // pass
            }

            try
            {
                ParticipantCommand::Kind participantCommand = ToParticipantCommand(cmdName);
                std::string participantName;
                cmdStream >> participantName;
                if (participantName.empty())
                {
                    if(participantCommand == ParticipantCommand::Kind::Shutdown)
                    {
                        for(auto&& name: _expectedParticipantNames)
                        {
                            SendCommand(participantCommand, name);
                        }
                    }
                    else
                    {
                        throw ib::TypeConversionError{"Participant command requrires a participant name"};
                    }
                }

                SendCommand(participantCommand, participantName);
                std::cout << "> ";
                continue;
            }
            catch (const ib::TypeConversionError&)
            {
                // pass
            }

            std::cerr << "Invalid command: '" << cmdName << "'" << std::endl;
            std::cout << "Available system commands: 'Run', 'Stop'" << std::endl;
            std::cout << "Available participant commands: 'Initialize <participantName>',"
                << " 'Restart <participantName>', 'Shutdown <participantname>'"
                << std::endl;
            std::cout << "Command 'Exit' ends the process" << std::endl;
            std::cout << "> ";
        }

    }

    void SendCommand(SystemCommand::Kind systemCommand)
    {
        std::cout << "Sending SystemCommand::" << systemCommand << std::endl;
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
        case SystemCommand::Kind::AbortSimulation: 
            _controller->AbortSimulation();
            return;
        }
    }

    void SendCommand(ParticipantCommand::Kind participantCommand, std::string participantName)
    {
        std::cout << "Sending ParticipantCommand::" << participantCommand << " to participant '" << participantName << "'" << std::endl;
        switch (participantCommand)
        {
        case ParticipantCommand::Kind::Invalid:
            return;
        case ParticipantCommand::Kind::Initialize:
            _controller->Initialize(participantName);
            return;
        case ParticipantCommand::Kind::Restart:
            _controller->Restart(participantName);
        case ParticipantCommand::Kind::Shutdown:
            _controller->Shutdown(participantName);
        }
    }

private:
    std::shared_ptr<ib::cfg::IParticipantConfiguration> _participantConfiguration;
    std::vector<std::string> _expectedParticipantNames;
    std::string _myParticipantName;
    ISystemController* _controller{nullptr};
};

int main(int argc, char** argv)
{
    ib::util::CommandlineParser commandlineParser;
    commandlineParser.Add<ib::util::CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<ib::util::CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("domain", "d", "42", "[--domain <domainId>]",
        "-d, --domain <domainId>: The domain ID that is used by the Integration Bus. Defaults to 42.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("name", "n", "SystemController", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'SystemController'.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.");
    commandlineParser.Add<ib::util::CommandlineParser::PositionalList>("participantNames", "<participantName1> [<participantName2> ...]",
        "<participantName1>, <participantName2>, ...: Names of participants to wait for before starting simulation.");

    std::cout << "Vector Integration Bus (VIB) -- Interactive System Controller" << std::endl
        << std::endl;

    try
    {
        commandlineParser.ParseArguments(argc, argv);
    }
    catch (std::runtime_error & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    if (commandlineParser.Get<ib::util::CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<ib::util::CommandlineParser::Flag>("version").Value())
    {
        std::string ibHash{ ib::version::GitHash() };
        auto ibShortHash = ibHash.substr(0, 7);
        std::cout
            << "Version Info:" << std::endl
            << " - Vector Integration Bus (VIB): " << ib::version::String() << ", #" << ibShortHash << std::endl;

        return 0;
    }

    if (!commandlineParser.Get<ib::util::CommandlineParser::PositionalList>("participantNames").HasValues())
    {
        std::cerr << "Error: Arguments '<participantName1> [<participantName2> ...]' are missing" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    auto domain{ commandlineParser.Get<ib::util::CommandlineParser::Option>("domain").Value() };
    auto participantName{ commandlineParser.Get<ib::util::CommandlineParser::Option>("name").Value() };
    auto configurationFilename{ commandlineParser.Get<ib::util::CommandlineParser::Option>("configuration").Value() };
    auto expectedParticipantNames{ commandlineParser.Get<ib::util::CommandlineParser::PositionalList>("participantNames").Values() };

    uint32_t domainId;
    try
    {
        domainId = static_cast<uint32_t>(std::stoul(domain));
    }
    catch (std::exception&)
    {
        std::cerr << "Error: Domain '" << domain << "' is not a valid number" << std::endl;

        return -1;
    }

    std::shared_ptr<ib::cfg::IParticipantConfiguration> configuration;
    try
    {
        configuration = !configurationFilename.empty() ?
            ib::cfg::ParticipantConfigurationFromFile(configurationFilename) :
            ib::cfg::ParticipantConfigurationFromString("");
    }
    catch (const ib::ConfigurationError & error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -2;
    }

    try
    {
        std::cout << "Creating participant '" << participantName << "' at domain " << domainId << ", expecting ";
        std::cout << (expectedParticipantNames.size() > 1 ? "participants '" : "participant '");
        std::copy(expectedParticipantNames.begin(), std::prev(expectedParticipantNames.end()), std::ostream_iterator<std::string>(std::cout, "', '"));
        std::cout << expectedParticipantNames.back() << "'..." << std::endl;

        auto participant = ib::CreateParticipant(configuration, participantName, domainId);

        auto systemMonitor = participant->GetSystemMonitor();
        auto systemController = participant->GetSystemController();
        systemController->SetWorkflowConfiguration({expectedParticipantNames});
        systemMonitor->AddParticipantStatusHandler(&ReportParticipantStatus);
        systemMonitor->AddSystemStateHandler(&ReportSystemState);

        InteractiveSystemController interactiveSystemController(participant->GetSystemController(), configuration, participantName, expectedParticipantNames);
        interactiveSystemController.RunInteractiveLoop();
    }
    catch (const std::exception & error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -3;
    }

    return 0;
}
