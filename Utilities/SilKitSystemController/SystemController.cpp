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
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "CommandlineParser.hpp"

using namespace SilKit;
using namespace SilKit::Services::Orchestration;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

class SilKitController
{
public:
    SilKitController(SilKit::IParticipant* participant,
                     std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                     const std::vector<std::string>& expectedParticipantNames)
        : _config{std::move(config)}
        , _expectedParticipantNames{expectedParticipantNames}
    {
        _controller = SilKit::Experimental::Participant::CreateSystemController(participant);
        _controller->SetWorkflowConfiguration({expectedParticipantNames});
        _monitor = participant->CreateSystemMonitor();

        _lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        _finalStatePromise = _lifecycleService->StartLifecycle();
    }

    void StopOrAbort()
    {
        if (_monitor->SystemState() == SystemState::Running || _monitor->SystemState() == SystemState::Paused)
        {
            std::cout << "Stopping the SIL Kit simulation..." << std::endl;
            _lifecycleService->Stop("Stop via interaction in sil-kit-system-controller");
        }
        else if (_monitor->SystemState() == SystemState::Aborting)
        {
            std::cout << "SIL Kit simulation is already aborting..." << std::endl;
            _aborted = true;
        }
        else if (_monitor->SystemState() == SystemState::Shutdown)
        {
            _externalShutdown = true;
        }
        else
        {
            std::cout << "SIL Kit SystemState is invalid. Sending AbortSimulation..." << std::endl;
            _controller->AbortSimulation();
            _aborted = true;
        }
    }

    void WaitForFinalStateWithRetries()
    {
        const int numRetries = 3;
        for (int retries = 1; retries <= numRetries; retries++)
        {
            auto status = _finalStatePromise.wait_for(5s);
            if (status == std::future_status::ready)
            {
                if (_aborted)
                {
                    std::cout << "SIL Kit simulation shut down after AbortSimulation." << std::endl;
                }
                else if (_externalShutdown)
                {
                    std::cout << "SIL Kit simulation was shut down externally." << std::endl;
                }
                else
                {
                    std::cout << "SIL Kit simulation shut down." << std::endl;
                }
                return;
            }
            else
            {
                std::cout << "SIL Kit simulation did not shut down in 5s... Retry " << retries << "/" << numRetries << std::endl;
            }
        }

        if (_aborted)
        {
            std::cout << "SIL Kit simulation did not shut down after AbortSimulation. Terminating." << std::endl;
            return;
        }
        else
        {
            std::cout << "SIL Kit simulation did not shut down after Stop. Sending AbortSimulation..." << std::endl;
            _aborted = true;
            _controller->AbortSimulation();
            WaitForFinalStateWithRetries();
        }
    }

    void WaitForExternalShutdown()
    {
        ParticipantState state = _finalStatePromise.get();
        if (state == ParticipantState::Shutdown)
        {
            std::cout << "SIL Kit simulation was shut down externally." << std::endl;
        }
        else
        {
            std::cerr << "Warning: Exited with an unexpected participant state: " << state << std::endl;
        }
    }

private:
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> _config;
    std::vector<std::string> _expectedParticipantNames;

    bool _aborted = false;
    bool _externalShutdown = false;
    SilKit::Experimental::Services::Orchestration::ISystemController* _controller;
    ISystemMonitor* _monitor;
    ILifecycleService* _lifecycleService;
    std::future<ParticipantState> _finalStatePromise;
};

int main(int argc, char** argv)
{
    using namespace SilKit::Util;
    CommandlineParser commandlineParser;
    commandlineParser.Add<CommandlineParser::Flag>("version", "v", "[--version]", "-v, --version: Get version info.");
    commandlineParser.Add<CommandlineParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");
    commandlineParser.Add<CommandlineParser::Flag>(
        "non-interactive", "ni", "[--non-interactive]",
        "--non-interactive: Run without awaiting any user interactions at any time.");
    commandlineParser.Add<CommandlineParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CommandlineParser::Option>("name", "n", "SystemController", "[--name <participantName>]",
                                                     "-n, --name <participantName>: The participant name used to take "
                                                     "part in the simulation. Defaults to 'SystemController'.");
    commandlineParser.Add<CommandlineParser::Option>(
        "configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. "
        "Note that the format was changed in v3.6.11.");
    commandlineParser.Add<CommandlineParser::PositionalList>(
        "participantNames", "<participantName1> [<participantName2> ...]",
        "<participantName1>, <participantName2>, ...: Names of participants to wait for before starting simulation.");

    std::cout << "Vector SIL Kit -- System Controller, SIL Kit version: " << SilKit::Version::String() << std::endl
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

    if (commandlineParser.Get<CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<CommandlineParser::Flag>("version").Value())
    {
        std::string hash{SilKit::Version::GitHash()};
        auto shortHash = hash.substr(0, 7);
        std::cout << "Version Info:" << std::endl
                  << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    if (!commandlineParser.Get<CommandlineParser::PositionalList>("participantNames").HasValues())
    {
        std::cerr << "Error: Arguments '<participantName1> [<participantName2> ...]' are missing" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    auto participantName{commandlineParser.Get<CommandlineParser::Option>("name").Value()};
    auto configurationFilename{commandlineParser.Get<CommandlineParser::Option>("configuration").Value()};
    auto expectedParticipantNames{
        commandlineParser.Get<CommandlineParser::PositionalList>("participantNames").Values()};
    auto connectUri{commandlineParser.Get<CommandlineParser::Option>("connect-uri").Value()};

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
        std::cout << "Creating participant '" << participantName << "' with registry " << connectUri
                  << ", expecting participant" << (expectedParticipantNames.size() > 1 ? "s '" : " '");
        std::copy(expectedParticipantNames.begin(), std::prev(expectedParticipantNames.end()),
                  std::ostream_iterator<std::string>(std::cout, "', '"));
        std::cout << expectedParticipantNames.back() << "'..." << std::endl;

        auto participant = SilKit::CreateParticipant(configuration, participantName, connectUri);

        expectedParticipantNames.push_back(participantName);
        SilKitController controller(participant.get(), configuration, expectedParticipantNames);

        bool nonInteractiveMode =
            (commandlineParser.Get<CommandlineParser::Flag>("non-interactive").Value()) ? true : false;

        if (nonInteractiveMode)
        {
            controller.WaitForExternalShutdown();
        }
        else
        {
            std::cout << "Press enter to stop the SIL Kit simulation..." << std::endl;
            std::cin.ignore();

            controller.StopOrAbort();
            controller.WaitForFinalStateWithRetries();
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
