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

#include "silkit/SilKitVersion.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "CommandlineParser.hpp"

using namespace SilKit;
using namespace SilKit::Services::Orchestration;

using namespace std::chrono_literals;

SystemCommand::Kind ToSystemCommand(const std::string& cmdString)
{
    if (cmdString == "Invalid")
        return SystemCommand::Kind::Invalid;
    else if (cmdString == "Run")
        return SystemCommand::Kind::Run;
    else if (cmdString == "Stop")
        return SystemCommand::Kind::Stop;
    else if (cmdString == "Abort" || cmdString == "AbortSimulation")
        return SystemCommand::Kind::AbortSimulation;

    throw SilKit::TypeConversionError{};
}

void ReportParticipantStatus(const SilKit::Services::Orchestration::ParticipantStatus& status)
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

void ReportSystemState(SilKit::Services::Orchestration::SystemState state)
{
    std::cout << "New SystemState: " << state << std::endl;
}

ParticipantCommand::Kind ToParticipantCommand(const std::string& cmdString)
{
    if (cmdString == "Invalid")
        return ParticipantCommand::Kind::Invalid;
    else if (cmdString == "Restart")
        return ParticipantCommand::Kind::Restart;
    else if (cmdString == "Shutdown")
        return ParticipantCommand::Kind::Shutdown;

    throw SilKit::TypeConversionError{};
}

class InteractiveSystemController
{
public:
    InteractiveSystemController(ISystemController* controller,
                                std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration, std::string myName,
                                std::vector<std::string> expectedParticipantNames)
        : _participantConfiguration{std::move(participantConfiguration)}
        , _expectedParticipantNames{std::move(expectedParticipantNames)}
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
            catch (const SilKit::TypeConversionError&)
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
                        throw SilKit::TypeConversionError{"Participant command requrires a participant name"};
                    }
                }

                SendCommand(participantCommand, participantName);
                std::cout << "> ";
                continue;
            }
            catch (const SilKit::TypeConversionError&)
            {
                // pass
            }

            std::cerr << "Invalid command: '" << cmdName << "'" << std::endl;
            std::cout << "Available system commands: 'Run', 'Stop', 'Abort'" << std::endl;
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
            break;
        case SystemCommand::Kind::Run:
            _controller->Run();
            break;
        case SystemCommand::Kind::Stop:
            _controller->Stop();
            break;
        case SystemCommand::Kind::AbortSimulation:
            _controller->AbortSimulation();
            break;
        }
    }

    void SendCommand(ParticipantCommand::Kind participantCommand, const std::string& participantName)
    {
        std::cout << "Sending ParticipantCommand::" << participantCommand << " to participant '" << participantName << "'" << std::endl;
        switch (participantCommand)
        {
        case ParticipantCommand::Kind::Invalid:
            break;
        case ParticipantCommand::Kind::Restart:
            _controller->Restart(participantName);
            break;
        case ParticipantCommand::Kind::Shutdown:
            _controller->Shutdown(participantName);
            break;
        }
    }

private:
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> _participantConfiguration;
    std::vector<std::string> _expectedParticipantNames;
    std::string _myParticipantName;
    ISystemController* _controller{nullptr};
};

int main(int argc, char** argv)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "SystemController", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'SystemController'.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::PositionalList>("participantNames", "<participantName1> [<participantName2> ...]",
        "<participantName1>, <participantName2>, ...: Names of participants to wait for before starting simulation.");

    std::cout << "Vector SilKit -- Interactive System Controller" << std::endl
        << std::endl;

    try
    {
        commandlineParser.ParseArguments(argc, argv);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    if (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value())
    {
        std::string hash{ SilKit::Version::GitHash() };
        auto shortHash = hash.substr(0, 7);
        std::cout
            << "Version Info:" << std::endl
            << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    if (!commandlineParser.Get<SilKit::Util::CommandlineParser::PositionalList>("participantNames").HasValues())
    {
        std::cerr << "Error: Arguments '<participantName1> [<participantName2> ...]' are missing" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    auto connectUri{ commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("connect-uri").Value() };
    auto participantName{ commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value() };
    auto configurationFilename{ commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("configuration").Value() };
    auto expectedParticipantNames{ commandlineParser.Get<SilKit::Util::CommandlineParser::PositionalList>("participantNames").Values() };

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    try
    {
        configuration = !configurationFilename.empty() ?
            SilKit::Config::ParticipantConfigurationFromFile(configurationFilename) :
            SilKit::Config::ParticipantConfigurationFromString("");
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -2;
    }

    try
    {
        std::cout << "Creating participant '" << participantName << "' with registry " << connectUri << ", expecting ";
        std::cout << (expectedParticipantNames.size() > 1 ? "participants '" : "participant '");
        std::copy(expectedParticipantNames.begin(), std::prev(expectedParticipantNames.end()), std::ostream_iterator<std::string>(std::cout, "', '"));
        std::cout << expectedParticipantNames.back() << "'..." << std::endl;

        auto participant = SilKit::CreateParticipant(configuration, participantName, connectUri);

        auto systemMonitor = participant->GetSystemMonitor();
        auto systemController = participant->GetSystemController();
        systemController->SetWorkflowConfiguration({expectedParticipantNames});
        systemMonitor->AddParticipantStatusHandler(&ReportParticipantStatus);
        systemMonitor->AddSystemStateHandler(&ReportSystemState);

        InteractiveSystemController interactiveSystemController(participant->GetSystemController(), configuration, participantName, expectedParticipantNames);
        interactiveSystemController.RunInteractiveLoop();
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
