// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#define _CRT_SECURE_NO_WARNINGS 1

#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <thread>

#include "silkit/SilKitVersion.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "CommandlineParser.hpp"
#include "SignalHandler.hpp"

using namespace SilKit;
using namespace SilKit::Util;
using namespace std::chrono_literals;

using CliParser = SilKit::Util::CommandlineParser;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

auto getConfig(const std::string& configurationFilename, const std::string& logLevel)
{
    if (!configurationFilename.empty())
    {
        if (!logLevel.empty())
        {
            std::cout << "warning: loglevel provided via commandline will be ignored in favor of the provided "
                         "participant config!"
                      << std::endl;
        }
        return SilKit::Config::ParticipantConfigurationFromFile(configurationFilename);
    }
    else
    {
        std::stringstream defaultConfig;
        defaultConfig << R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":")" << logLevel << R"("}]}})";
        return SilKit::Config::ParticipantConfigurationFromString(defaultConfig.str());
    }
}

int main(int argc, char** argv)
{
    CliParser commandlineParser;
    commandlineParser.Add<CliParser::Flag>("version", "v", "[--version]", "-v, --version: Get version info.");
    commandlineParser.Add<CliParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");
    commandlineParser.Add<CliParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CliParser::Option>(
        "name", "n", "SystemMonitor", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to "
        "'SystemMonitor'.");
    commandlineParser.Add<CliParser::Option>("loglevel", "l", "",
                                             "[--loglevel <Critical|Error|Warning|Info|Debug|Trace>]",
                                             "-l, --loglevel <Critical|Error|Warning|Info|Debug|Trace>: The log level "
                                             "used for the sil-kit-monitor. Defaults to "
                                             "Info.");
    commandlineParser.Add<CliParser::Option>("configuration", "c", "", "[--configuration <filePath>]",
                                             "-c, --configuration <filePath>: Path to the Participant configuration "
                                             "YAML or JSON file. Note that the format was changed in v3.6.11.");
    commandlineParser.Add<CliParser::Flag>("autonomous", "a", "[--autonomous]",
                                           "-a, --autonomous: Run with an autonomous lifecycle.");
    commandlineParser.Add<CliParser::Flag>("coordinated", "r", "[--coordinated]",
                                           "-r, --coordinated: Run with a coordinated lifecycle.");
    commandlineParser.Add<CliParser::Flag>("sync", "s", "[--sync]",
                                           "-s, --sync: Run with virtual time synchronization.");

    std::cout << "Vector SIL Kit -- System Monitor, SIL Kit version: " << SilKit::Version::String() << std::endl
              << std::endl;

    try
    {
        commandlineParser.ParseArguments(argc, argv);
    }
    catch (const SilKit::SilKitError& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    if (commandlineParser.Get<CliParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<CliParser::Flag>("version").Value())
    {
        std::string hash{SilKit::Version::GitHash()};
        auto shortHash = hash.substr(0, 7);
        std::cout << "Version Info:" << std::endl
                  << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    const auto connectUri{commandlineParser.Get<CliParser::Option>("connect-uri").Value()};
    const auto participantName{commandlineParser.Get<CliParser::Option>("name").Value()};
    const auto configurationFilename{commandlineParser.Get<CliParser::Option>("configuration").Value()};
    const auto logLevel = commandlineParser.Get<CliParser::Option>("loglevel").HasValue()
                              ? commandlineParser.Get<CliParser::Option>("loglevel").Value()
                              : "Info";

    bool autonomousMode = (commandlineParser.Get<CliParser::Flag>("autonomous").Value()) ? true : false;
    bool coordinatedMode = (commandlineParser.Get<CliParser::Flag>("coordinated").Value()) ? true : false;
    bool sync = (commandlineParser.Get<CliParser::Flag>("sync").Value()) ? true : false;

    if (autonomousMode && coordinatedMode)
    {
        std::cerr << "Invalid command line arguments. Choose either autonomous or coordinated mode." << std::endl;
        return -1;
    }
    if (sync && !coordinatedMode && !autonomousMode)
    {
        std::cerr
            << "Invalid command line arguments. Time synchonization requires either autonomous or coordinated mode."
            << std::endl;
        return -1;
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    try
    {
        configuration = getConfig(configurationFilename, logLevel);
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what()
                  << std::endl;

        return -2;
    }

    try
    {
        std::cout << "Creating participant '" << participantName << "' with registry " << connectUri << std::endl;

        auto participant = SilKit::CreateParticipant(std::move(configuration), participantName, connectUri);

        auto* logger = participant->GetLogger();
        auto* systemMonitor = participant->CreateSystemMonitor();

        systemMonitor->SetParticipantConnectedHandler(
            [logger](const Services::Orchestration::ParticipantConnectionInformation& status) {
            std::stringstream buffer;
            buffer << "New Participant: \'" << status.participantName << "\'" << std::endl;
            logger->Info(buffer.str());
        });

        systemMonitor->AddParticipantStatusHandler([logger](const Services::Orchestration::ParticipantStatus& status) {
            std::stringstream buffer;
            buffer << "New ParticipantState of \'" << status.participantName << "\': " << status.state
                   << ", reason: " << status.enterReason;
            logger->Info(buffer.str());
        });

        systemMonitor->AddSystemStateHandler([logger](Services::Orchestration::SystemState state) {
            std::stringstream buffer;
            buffer << "New SystemState: " << state;
            logger->Info(buffer.str());
        });

        if (autonomousMode || coordinatedMode)
        {
            auto opMode = autonomousMode ? Services::Orchestration::OperationMode::Autonomous
                                         : Services::Orchestration::OperationMode::Coordinated;

            auto* lifecycle = participant->CreateLifecycleService({opMode});

            if (sync)
            {
                auto* timeSyncService = lifecycle->CreateTimeSyncService();
                timeSyncService->SetSimulationStepHandler([logger](auto now, auto) {
                    std::stringstream buffer;
                    buffer << "now=" << now;
                    logger->Info(buffer.str());
                }, 1ms);
            }
            auto finalStateFuture = lifecycle->StartLifecycle();

            std::cout << "Press Ctrl-C to terminate..." << std::endl;

            std::promise<int> signalPromise;
            auto signalValue = signalPromise.get_future();
            RegisterSignalHandler([&](auto sigNum) { signalPromise.set_value(sigNum); });
            signalValue.wait();

            {
                std::stringstream buffer;
                buffer << "Signal " << signalValue.get() << " received, exiting...";
                logger->Info(buffer.str());
            }
            lifecycle->Stop("Stopping the System Monitor");
        }
        else
        {
            std::cout << "Press Ctrl-C to terminate..." << std::endl;

            std::promise<int> signalPromise;
            auto signalValue = signalPromise.get_future();
            RegisterSignalHandler([&](auto sigNum) { signalPromise.set_value(sigNum); });
            signalValue.wait();

            {
                std::stringstream buffer;
                buffer << "Signal " << signalValue.get() << " received, exiting...";
                logger->Info(buffer.str());
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;

        return -3;
    }

    return 0;
}
