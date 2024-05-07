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
#include <cctype>
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
#include "SignalHandler.hpp"

using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::Logging;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

namespace {

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
        _logger = participant->GetLogger();

        _monitor->SetParticipantConnectedHandler(
            [this](const ParticipantConnectionInformation& participantInformation) {
                std::ostringstream ss;
                ss << "Participant '" << participantInformation.participantName << "' joined the simulation";
                LogInfo(ss.str());
            });
        _monitor->AddSystemStateHandler([&](const SystemState& systemState) {
            if (systemState == SystemState::Stopping)
            {
                if (!_isStopRequested)
                {
                    LogInfo("Another participant causes the simulation to stop");
                }
            }
            else if (systemState == SystemState::Error)
            {
                LogInfo("Simulation is in error state");
            }
        });

        _lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        auto finalStatePromise = _lifecycleService->StartLifecycle();
        _finalStatePromise = finalStatePromise.share();
    }

    void RegisterSignalHandler()
    {
        SilKit::Util::RegisterSignalHandler([&](auto signalValue) {
            {
                std::ostringstream ss;
                ss << "Signal " << signalValue << " received, attempting to stop simulation...";
                LogInfo(ss.str());
            }
            StopOrAbort();
            WaitForFinalStateWithRetries();
        });
    }

    void WaitForFinalState()
    {
        ParticipantState state = _finalStatePromise.get();
        if (state != ParticipantState::Shutdown)
        {
            std::ostringstream ss;
            ss << "Simulation ended with an unexpected participant state: " << state;
            LogWarn(ss.str());
        }
        else
        {
            LogInfo("Simulation ended, SystemController is shut down.");
        }
    }

private:
    void StopOrAbort()
    {
        _isStopRequested = true;
        if (_monitor->SystemState() == SystemState::Running || _monitor->SystemState() == SystemState::Paused)
        {
            LogInfo("System controller stops the simulation...");
            _lifecycleService->Stop("Stop via interaction in sil-kit-system-controller");
        }
        else if (_monitor->SystemState() == SystemState::Aborting)
        {
            LogWarn("Simulation is already aborting...");
            _aborted = true;
        }
        else if (_monitor->SystemState() != SystemState::Shutdown)
        {
            {
                std::ostringstream ss;
                ss << "Simulation is in state " << _monitor->SystemState() << " and cannot be stopped, attempting to abort...";
                LogInfo(ss.str());
            }
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
                return;
            }
            else
            {
                std::ostringstream ss;
                ss << "Simulation did not shut down in 5s... Retry " << retries << "/" << numRetries;
                LogWarn(ss.str());
            }
        }

        if (_aborted)
        {
            LogWarn("Simulation did not shut down via abort signal. Terminating...");
            _lifecycleService->ReportError("Simulation did not shut down via abort signal");
        }
        else
        {
            LogWarn("Simulation did not shut down via stop signal. Attempting to abort...");
            _aborted = true;
            _controller->AbortSimulation();
            WaitForFinalStateWithRetries();
        }
    }

    void LogInfo(const std::string& message)
    {
        _logger->Info(message);
    }

    void LogWarn(const std::string& message)
    {
        _logger->Warn(message);
    }

    void LogDebug(const std::string& message)
    {
        _logger->Debug(message);
    }

private:
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> _config;
    std::vector<std::string> _expectedParticipantNames;

    std::atomic<bool> _isStopRequested{false};
    bool _aborted = false;
    SilKit::Experimental::Services::Orchestration::ISystemController* _controller;
    ISystemMonitor* _monitor;
    ILifecycleService* _lifecycleService;
    ILogger* _logger;
    std::shared_future<ParticipantState> _finalStatePromise;
};

auto ToLowerCase(std::string s) -> std::string
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return (unsigned char)std::tolower(c);
    });
    return s;
}

auto IsValidLogLevel(const std::string& levelStr) -> bool
{
    auto logLevel = ToLowerCase(levelStr);
    return    logLevel == "trace"
           || logLevel == "debug"
           || logLevel == "warn"
           || logLevel == "info"
           || logLevel == "error"
           || logLevel == "critical"
           || logLevel == "off";
}

} // namespace

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
        "Note that the format was changed in v3.6.11. Cannot be used together with the '--log' option.");
    commandlineParser.Add<CommandlineParser::Option>(
        "log", "l", "info", "[--log <level>]",
        "-l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. "
        "Defaults to 'info' if the '--configuration' option is not specified. Cannot be used together with the "
        "'--configuration' option.");
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

    const bool hasLogOption{commandlineParser.Get<CommandlineParser::Option>("log").HasValue()};
    const bool hasCfgOption{commandlineParser.Get<CommandlineParser::Option>("configuration").HasValue()};
    if (hasLogOption && hasCfgOption)
    {
        std::cerr << "Error: Options '--log' and '--configuration' cannot be used simultaneously" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);
        return -1;
    }

    auto configurationFilename{commandlineParser.Get<CommandlineParser::Option>("configuration").Value()};
    auto expectedParticipantNames{
        commandlineParser.Get<CommandlineParser::PositionalList>("participantNames").Values()};
    auto connectUri{commandlineParser.Get<CommandlineParser::Option>("connect-uri").Value()};

    const auto logLevel{commandlineParser.Get<CommandlineParser::Option>("log").Value()};
    if (!IsValidLogLevel(logLevel))
    {
        std::cerr << "Error: Argument of the '--log' option must be one of 'trace', 'debug', 'warn', 'info', 'error', "
                     "'critical', or 'off'"
                  << std::endl;
        return -1;
    }

    const bool nonInteractiveMode =
        (commandlineParser.Get<CommandlineParser::Flag>("non-interactive").Value()) ? true : false;

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    try
    {
        if (hasCfgOption)
        {
            configuration = SilKit::Config::ParticipantConfigurationFromFile(configurationFilename);
        }
        else
        {
            std::string configLogLevel{logLevel};

            // due to the validation, we know that the first character is ASCII
            configLogLevel[0] = static_cast<char>(std::toupper(configLogLevel[0]));

            std::ostringstream ss;
            ss << R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":")" << configLogLevel << R"("}]}})";

            configuration = SilKit::Config::ParticipantConfigurationFromString(ss.str());
        }
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

        auto participant = SilKit::CreateParticipant(configuration, participantName, connectUri);

        auto* logger{participant->GetLogger()};
        {
            std::ostringstream ss;
            ss << "System controller is expecting " << expectedParticipantNames.size() << " participant"
               << (expectedParticipantNames.size() > 1 ? "s" : "") << ": '";
            std::copy(expectedParticipantNames.begin(), std::prev(expectedParticipantNames.end()),
                      std::ostream_iterator<std::string>(ss, "', '"));
            ss << expectedParticipantNames.back() << "'";
            logger->Info(ss.str());
        }

        expectedParticipantNames.push_back(participantName);
        SilKitController controller(participant.get(), configuration, expectedParticipantNames);

        if (!nonInteractiveMode)
        {
            std::cout << "Press Ctrl-C to end the simulation..." << std::endl;
        }
        controller.RegisterSignalHandler();
        controller.WaitForFinalState();

        if (!nonInteractiveMode)
        {
            std::cout << "Press enter to end the process..." << std::endl;
            std::cin.ignore();
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        if (!nonInteractiveMode)
        {
                std::cout << "Press enter to end the process..." << std::endl;
                std::cin.ignore();
        }

        return -3;
    }

    return 0;
}
