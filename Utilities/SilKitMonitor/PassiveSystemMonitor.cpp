/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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

using namespace SilKit;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

int main(int argc, char** argv)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
                                                                 "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");

    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "name", "n", "SystemMonitor", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to "
        "'SystemMonitor'.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("autonomous", "a", "[--autonomous]",
                                                                 "-a, --autonomous: Run with an autonomous lifecycle");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("coordinated", "r", "[--coordinated]",
                                                                 "-r, --coordinated: Run with a coordinated lifecycle");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("sync", "s", "[--sync]",
                                                                 "-s, --sync: Run with virtual time synchronization");

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

    if (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value())
    {
        std::string hash{SilKit::Version::GitHash()};
        auto shortHash = hash.substr(0, 7);
        std::cout << "Version Info:" << std::endl
                  << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    auto connectUri{commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("connect-uri").Value()};
    auto participantName{commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value()};
    auto configurationFilename{commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("configuration").Value()};

    bool autonomousMode =
        (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("autonomous").Value()) ? true : false;
    bool coordinatedMode =
        (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("coordinated").Value()) ? true : false;
    bool sync = (commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("sync").Value()) ? true : false;

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
        configuration = !configurationFilename.empty()
                            ? SilKit::Config::ParticipantConfigurationFromFile(configurationFilename)
                            : SilKit::Config::ParticipantConfigurationFromString("");
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what()
                  << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -2;
    }

    try
    {
        std::cout << "Creating participant '" << participantName << "' with registry " << connectUri << std::endl;

        auto participant = SilKit::CreateParticipant(std::move(configuration), participantName, connectUri);

        auto* logger = participant->GetLogger();
        auto* systemMonitor = participant->CreateSystemMonitor();

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

        std::cout << "Press enter to terminate the SystemMonitor..." << std::endl;
        if (autonomousMode || coordinatedMode)
        {
            auto opMode = autonomousMode ? Services::Orchestration::OperationMode::Autonomous
                                         : Services::Orchestration::OperationMode::Coordinated;

            auto* lifecycle = participant->CreateLifecycleService({opMode});

            if (sync)
            {
                auto* timeSyncService = lifecycle->CreateTimeSyncService();
                timeSyncService->SetSimulationStepHandler(
                    [logger](auto now, auto) {
                        std::stringstream buffer;
                        buffer << "now=" << now;
                        logger->Info(buffer.str());
                    },
                    1ms);
            }
            auto finalStateFuture = lifecycle->StartLifecycle();

            std::cin.ignore();

            lifecycle->Stop("Stopping the SystemMonitor");
            auto finalState = finalStateFuture.get();
            std::cout << "SystemMonitor stopped. Final State: " << finalState << std::endl;
        }
        else
        {
            std::cin.ignore();
        }
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
