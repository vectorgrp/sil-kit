// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
#include <set>

#include "silkit/SilKitVersion.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "CommandlineParser.hpp"
#include "SignalHandler.hpp"

using namespace SilKit;
using namespace SilKit::Util;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::Logging;

using CliParser = SilKit::Util::CommandlineParser;

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
                     const std::set<std::string>& requiredParticipantNames, const std::string& systemControllerName)
        : _config{std::move(config)}
        , _requiredParticipantNames{requiredParticipantNames}
    {
        _controller = SilKit::Experimental::Participant::CreateSystemController(participant);
        _monitor = participant->CreateSystemMonitor();
        _logger = participant->GetLogger();

        _monitor->SetParticipantConnectedHandler(
            [this, systemControllerName](const ParticipantConnectionInformation& participantInformation) {
            std::ostringstream ss;
            ss << "Participant '" << participantInformation.participantName << "' connected.";
            LogInfo(ss.str());

            if (_workflowConfigSet)
            {
                return;
            }

            if (_requiredParticipantNames.count(participantInformation.participantName) == 0)
            {
                return;
            }

            _connectedParticipantNames.insert(participantInformation.participantName);

            if (_connectedParticipantNames == _requiredParticipantNames)
            {
                LogInfo("All required participants connected to initiate the coordinated simulation start.");

                std::vector<std::string> requiredParticipantNames(_requiredParticipantNames.begin(),
                                                                  _requiredParticipantNames.end());
                requiredParticipantNames.push_back(systemControllerName);
                _workflowConfigSet = true;
                _controller->SetWorkflowConfiguration({requiredParticipantNames});
            }
            else
            {
                LogRemainingRequiredParticipants();
            }
        });

        _monitor->SetParticipantDisconnectedHandler(
            [this](const ParticipantConnectionInformation& participantInformation) {
            std::ostringstream ss;
            ss << "Participant '" << participantInformation.participantName << "' disconnected.";
            LogInfo(ss.str());

            if (_workflowConfigSet)
            {
                return;
            }

            if (_requiredParticipantNames.count(participantInformation.participantName) > 0)
            {
                _connectedParticipantNames.erase(participantInformation.participantName);
                LogRemainingRequiredParticipants();
            }
        });

        _monitor->AddSystemStateHandler([&](const SystemState& systemState) {
            if (systemState == SystemState::Stopping)
            {
                if (!_isStopRequested)
                {
                    LogInfo("Another participant causes the simulation to stop.");
                }
            }
            else if (systemState == SystemState::Error)
            {
                LogInfo("Simulation is in error state. This requires restarting all required participants and the "
                        "System Controller.");
            }
        });

        _lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        auto finalStatePromise = _lifecycleService->StartLifecycle();
        _finalStatePromise = finalStatePromise.share();
    }

    void LogRemainingRequiredParticipants()
    {
        std::set<std::string> remaining;
        std::set_difference(_requiredParticipantNames.begin(), _requiredParticipantNames.end(),
                            _connectedParticipantNames.begin(), _connectedParticipantNames.end(),
                            std::inserter(remaining, remaining.begin()));
        if (!remaining.empty())
        {
            std::ostringstream ss;
            ss << "Waiting for participant" << (remaining.size() > 1 ? "s" : "") << ": ";
            bool first = true;
            for (const auto& name : remaining)
            {
                if (!first)
                    ss << ", ";
                ss << "'" << name << "'";
                first = false;
            }
            LogInfo(ss.str());
        }
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
            LogInfo("Simulation ended, System Controller is shut down.");
        }
    }

private:
    void StopOrAbort()
    {
        _isStopRequested = true;
        if (_monitor->SystemState() == SystemState::Running || _monitor->SystemState() == SystemState::Paused)
        {
            LogInfo("System Controller stops the simulation...");
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
                ss << "Simulation is in state " << _monitor->SystemState()
                   << " and cannot be stopped, attempting to abort...";
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
    const std::set<std::string> _requiredParticipantNames;
    std::set<std::string> _connectedParticipantNames;

    std::atomic<bool> _isStopRequested{false};
    bool _workflowConfigSet = false;
    bool _aborted = false;
    SilKit::Experimental::Services::Orchestration::ISystemController* _controller;
    ISystemMonitor* _monitor;
    ILifecycleService* _lifecycleService;
    ILogger* _logger;
    std::shared_future<ParticipantState> _finalStatePromise;
};

auto ToLowerCase(std::string s) -> std::string
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });
    return s;
}

auto IsValidLogLevel(const std::string& levelStr) -> bool
{
    auto logLevel = ToLowerCase(levelStr);
    return logLevel == "trace" || logLevel == "debug" || logLevel == "warn" || logLevel == "info" || logLevel == "error"
           || logLevel == "critical" || logLevel == "off";
}

} // namespace

int main(int argc, char** argv)
{
    CliParser commandlineParser;
    commandlineParser.Add<CliParser::Flag>("version", "v", "[--version]", "-v, --version: Get version info.");
    commandlineParser.Add<CliParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");
    commandlineParser.Add<CliParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CliParser::Option>("name", "n", "SystemController", "[--name <participantName>]",
                                             "-n, --name <participantName>: The participant name used to take "
                                             "part in the simulation. Defaults to 'SystemController'.");
    commandlineParser.Add<CliParser::Option>(
        "configuration", "c", "", "[--configuration <filePath>]",
        "-c, --configuration <filePath>: Path to the Participant configuration YAML or JSON file. "
        "Note that the format was changed in v3.6.11. Cannot be used together with the '--log' option.");
    commandlineParser.Add<CliParser::Option>(
        "log", "l", "info", "[--log <level>]",
        "-l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. "
        "Defaults to 'info' if the '--configuration' option is not specified. Cannot be used together with the "
        "'--configuration' option.");
    commandlineParser.Add<CliParser::PositionalList>(
        "participantNames", "<participantName1> [<participantName2> ...]",
        "<participantName1>, <participantName2>, ...: Names of participants to wait for before starting simulation.");

    // ignored and deprecated
    commandlineParser.Add<CliParser::Flag>("non-interactive", "ni", "[--non-interactive]",
                                           "--non-interactive: Run without awaiting any user interactions at any time.",
                                           CliParser::Hidden);

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

    if (!commandlineParser.Get<CliParser::PositionalList>("participantNames").HasValues())
    {
        std::cerr << "Error: Arguments '<participantName1> [<participantName2> ...]' are missing" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    const auto participantName{commandlineParser.Get<CliParser::Option>("name").Value()};
    const auto nonInteractiveMode = (commandlineParser.Get<CliParser::Flag>("non-interactive").Value());

    const bool hasLogOption{commandlineParser.Get<CliParser::Option>("log").HasValue()};
    const bool hasCfgOption{commandlineParser.Get<CliParser::Option>("configuration").HasValue()};
    if (hasLogOption && hasCfgOption)
    {
        std::cerr << "Error: Options '--log' and '--configuration' cannot be used simultaneously" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);
        return -1;
    }

    const auto configurationFilename{commandlineParser.Get<CliParser::Option>("configuration").Value()};
    const auto requiredParticipantNames{commandlineParser.Get<CliParser::PositionalList>("participantNames").Values()};
    const auto connectUri{commandlineParser.Get<CliParser::Option>("connect-uri").Value()};

    const auto logLevel{commandlineParser.Get<CliParser::Option>("log").Value()};
    if (!IsValidLogLevel(logLevel))
    {
        std::cerr << "Error: Argument of the '--log' option must be one of 'trace', 'debug', 'warn', 'info', 'error', "
                     "'critical', or 'off'"
                  << std::endl;
        return -1;
    }

    if (nonInteractiveMode)
    {
        std::cerr << "Warning: Flag '--non-interactive', '-ni' became obsolete with v4.0.53, the default behavior "
                     "is non-interactive since then."
                  << std::endl;
    }

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
            ss << "Coordinated simulation start requires " << requiredParticipantNames.size() << " participant"
               << (requiredParticipantNames.size() > 1 ? "s" : "") << ": '";
            std::copy(requiredParticipantNames.begin(), std::prev(requiredParticipantNames.end()),
                      std::ostream_iterator<std::string>(ss, "', '"));
            ss << requiredParticipantNames.back() << "'";
            logger->Info(ss.str());
        }

        const std::set<std::string> requiredParticipantNamesSet{requiredParticipantNames.begin(),
                                                                requiredParticipantNames.end()};

        if (requiredParticipantNamesSet.size() != requiredParticipantNames.size())
        {
            std::cerr << "Error: Duplicate name in list of required participants" << std::endl;
            return -2;
        }

        SilKitController controller(participant.get(), configuration, requiredParticipantNamesSet, participantName);

        std::cout << "Press Ctrl-C to end the simulation..." << std::endl;
        controller.RegisterSignalHandler();
        controller.WaitForFinalState();

        SilKit::Util::ShutdownSignalHandler();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;

        return -3;
    }

    return 0;
}
