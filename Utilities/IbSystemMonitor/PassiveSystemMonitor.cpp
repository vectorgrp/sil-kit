// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS 1

#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <thread>

#include "ib/version.hpp"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "CommandlineParser.hpp"

using namespace ib::mw;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

int main(int argc, char** argv)
{
    ib::util::CommandlineParser commandlineParser;
    commandlineParser.Add<ib::util::CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<ib::util::CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("domain", "d", "42", "[--domain <domainId>]",
        "-d, --domain <domainId>: The domain ID which is used by the Integration Bus. Defaults to 42.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("name", "n", "SystemMonitor", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'SystemMonitor'.");
    commandlineParser.Add<ib::util::CommandlineParser::Option>("configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration: Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.");

    std::cout << "Vector Integration Bus (VIB) -- System Monitor" << std::endl
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
            << " - Vector Integration Bus (VIB): " << ib::version::String() << " (" << ib::version::SprintName() << "), #" << ibShortHash << std::endl;

        return 0;
    }

    auto domain{ commandlineParser.Get<ib::util::CommandlineParser::Option>("domain").Value() };
    auto participantName{ commandlineParser.Get<ib::util::CommandlineParser::Option>("name").Value() };
    auto configurationFilename{ commandlineParser.Get<ib::util::CommandlineParser::Option>("configuration").Value() };

    int domainId;
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
        std::cout << "Creating participant '" << participantName << "' at domain " << domainId << std::endl;

        auto participant = ib::CreateSimulationParticipant(std::move(configuration), participantName, domainId, false);

        auto* logger = participant->GetLogger();
        auto* systemMonitor = participant->GetSystemMonitor();

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
