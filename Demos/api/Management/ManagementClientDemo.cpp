// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Experimental management client: shows the simulations on a registry with `Experimental.Management.Enable`, or
// prints the configuration a participant shared via `Experimental.Management.ShareConfiguration`.

#include <algorithm>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/experimental/management/ManagementClient.hpp"

#include "CommandlineParser.hpp"
#include "SignalHandler.hpp"

using namespace std::chrono_literals;

namespace Mgmt = SilKit::Experimental::Management;
using SilKit::Services::Rpc::RpcCallStatus;
using SilKit::Util::CommandlineParser;

namespace {

auto Join(const std::vector<std::string>& strings) -> std::string
{
    if (strings.empty())
    {
        return "-";
    }

    std::string joined;
    for (const auto& string : strings)
    {
        joined += (joined.empty() ? "" : ", ") + string;
    }
    return joined;
}

//! Participants that join without a simulation path in the registry URI share the simulation with the empty name.
auto DisplayName(const std::string& simulationName) -> std::string
{
    return simulationName.empty() ? "(default)" : simulationName;
}

auto DisplayNames(std::vector<std::string> simulationNames) -> std::vector<std::string>
{
    std::transform(simulationNames.begin(), simulationNames.end(), simulationNames.begin(), DisplayName);
    return simulationNames;
}

auto MissingParticipantNames(const Mgmt::SimulationInfo& simulation) -> std::vector<std::string>
{
    std::vector<std::string> missing;
    for (const auto& name : simulation.requiredParticipantNames)
    {
        if (std::find(simulation.participantNames.begin(), simulation.participantNames.end(), name)
            == simulation.participantNames.end())
        {
            missing.push_back(name);
        }
    }
    return missing;
}

auto CurrentTime() -> std::string
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    std::ostringstream out;
    out << std::put_time(&localTime, "%H:%M:%S");
    return out.str();
}

auto ToString(RpcCallStatus callStatus) -> std::string
{
    switch (callStatus)
    {
    case RpcCallStatus::Success:
        return "Success";
    case RpcCallStatus::ServerNotReachable:
        return "ServerNotReachable";
    case RpcCallStatus::UndefinedError:
        return "UndefinedError";
    case RpcCallStatus::InternalServerError:
        return "InternalServerError";
    case RpcCallStatus::Timeout:
        return "Timeout";
    }
    return "RpcCallStatus(" + std::to_string(static_cast<int>(callStatus)) + ")";
}

void PrintRegistryInfo(const std::string& registryUri, const Mgmt::RegistryInfo& registryInfo)
{
    std::cout << "registry " << registryUri << "  SIL Kit " << registryInfo.silKitVersion << "  API "
              << registryInfo.apiVersionMajor << "." << registryInfo.apiVersionMinor
              << "  features: " << Join(registryInfo.features) << std::endl;
}

void PrintSimulations(const Mgmt::Simulations& simulations, const Mgmt::SimulationsChange& change)
{
    std::cout << "\n[" << CurrentTime() << "] #" << simulations.sequence;
    if (!change.created.empty())
    {
        std::cout << "  created: " << Join(DisplayNames(change.created));
    }
    if (!change.removed.empty())
    {
        std::cout << "  removed: " << Join(DisplayNames(change.removed));
    }
    if (!change.changed.empty())
    {
        std::cout << "  changed: " << Join(DisplayNames(change.changed));
    }
    std::cout << "\n";

    if (simulations.simulations.empty())
    {
        std::cout << "no simulations" << std::endl;
        return;
    }

    const std::string simulationHeader{"SIMULATION"};
    const std::string participantsHeader{"PARTICIPANTS"};
    auto simulationWidth = simulationHeader.size();
    auto participantsWidth = participantsHeader.size();
    for (const auto& simulation : simulations.simulations)
    {
        simulationWidth = std::max(simulationWidth, DisplayName(simulation.name).size());
        participantsWidth = std::max(participantsWidth, Join(simulation.participantNames).size());
    }

    std::cout << std::left << std::setw(static_cast<int>(simulationWidth + 2)) << simulationHeader
              << std::setw(static_cast<int>(participantsWidth + 2)) << participantsHeader << "REQUIRED (MISSING)\n";
    for (const auto& simulation : simulations.simulations)
    {
        auto required = Join(simulation.requiredParticipantNames);
        const auto missing = MissingParticipantNames(simulation);
        if (!missing.empty())
        {
            required += " (" + Join(missing) + ")";
        }

        std::cout << std::setw(static_cast<int>(simulationWidth + 2)) << DisplayName(simulation.name)
                  << std::setw(static_cast<int>(participantsWidth + 2)) << Join(simulation.participantNames) << required
                  << "\n";
    }
    std::cout << std::flush;
}

//! State handed from the client handlers to the main thread, which prints it: handlers must not block.
struct WatchState
{
    std::mutex mutex;
    std::condition_variable condition;
    bool stopRequested{false};
    bool hasRegistryInfo{false};
    Mgmt::RegistryInfo registryInfo;
    std::vector<std::pair<Mgmt::Simulations, Mgmt::SimulationsChange>> updates;
};

void Watch(Mgmt::ManagementClient& client, WatchState& state, const std::string& registryUri)
{
    client.SetRegistryInfoHandler([&state](const Mgmt::RegistryInfo& registryInfo) {
        std::lock_guard<std::mutex> lock{state.mutex};
        state.registryInfo = registryInfo;
        state.hasRegistryInfo = true;
        state.condition.notify_all();
    });
    client.SetSimulationsHandler([&state](const Mgmt::Simulations& simulations, const Mgmt::SimulationsChange& change) {
        std::lock_guard<std::mutex> lock{state.mutex};
        state.updates.emplace_back(simulations, change);
        state.condition.notify_all();
    });

    RegisterSignalHandler([&state](int) {
        std::lock_guard<std::mutex> lock{state.mutex};
        state.stopRequested = true;
        state.condition.notify_all();
    });

    std::cout << "Watching the simulations on " << registryUri << ", press Ctrl-C to stop." << std::endl;

    const auto hintTime = std::chrono::steady_clock::now() + 2s;
    auto registryInfoSeen = false;
    auto hintPrinted = false;
    for (;;)
    {
        std::unique_lock<std::mutex> lock{state.mutex};
        state.condition.wait_for(
            lock, 500ms, [&state] { return state.stopRequested || state.hasRegistryInfo || !state.updates.empty(); });
        if (state.stopRequested)
        {
            break;
        }

        const auto hasRegistryInfo = state.hasRegistryInfo;
        const auto registryInfo = state.registryInfo;
        state.hasRegistryInfo = false;
        auto updates = std::move(state.updates);
        state.updates.clear();
        lock.unlock();

        if (hasRegistryInfo)
        {
            registryInfoSeen = true;
            PrintRegistryInfo(registryUri, registryInfo);
        }
        for (const auto& update : updates)
        {
            PrintSimulations(update.first, update.second);
        }
        if (!registryInfoSeen && !hintPrinted && std::chrono::steady_clock::now() > hintTime)
        {
            hintPrinted = true;
            std::cout << "No registry info yet; is 'Experimental.Management.Enable' set for the registry?" << std::endl;
        }
    }

    ShutdownSignalHandler();
}

int PrintParticipantConfiguration(Mgmt::ManagementClient& client, const std::string& simulationName,
                                  const std::string& participantName)
{
    // the client does not retry: right after joining, the server of the management participant may be unknown yet
    const auto retryUntil = std::chrono::steady_clock::now() + 5s;
    for (;;)
    {
        std::promise<std::pair<RpcCallStatus, Mgmt::ParticipantConfigurationReply>> promise;
        auto future = promise.get_future();
        client.GetParticipantConfiguration(
            simulationName, participantName, 5s,
            [&promise](RpcCallStatus callStatus, const Mgmt::ParticipantConfigurationReply& reply) {
            promise.set_value({callStatus, reply});
        });
        const auto result = future.get();

        if (result.first == RpcCallStatus::ServerNotReachable && std::chrono::steady_clock::now() < retryUntil)
        {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        if (result.first != RpcCallStatus::Success)
        {
            std::cerr << "Error: request failed with " << ToString(result.first)
                      << "; is 'Experimental.Management.Enable' set for the registry?" << std::endl;
            return -1;
        }

        switch (result.second.status)
        {
        case Mgmt::ParticipantConfigurationStatus::Available:
            std::cout << result.second.configurationJson << std::endl;
            return 0;
        case Mgmt::ParticipantConfigurationStatus::NotShared:
            std::cerr << "Error: participant '" << participantName << "' in simulation '" << DisplayName(simulationName)
                      << "' did not set 'Experimental.Management.ShareConfiguration'" << std::endl;
            return -1;
        case Mgmt::ParticipantConfigurationStatus::UnknownParticipant:
            std::cerr << "Error: no participant '" << participantName << "' in simulation '"
                      << DisplayName(simulationName) << "'" << std::endl;
            return -1;
        }
        std::cerr << "Error: unknown participant configuration status" << std::endl;
        return -1;
    }
}

bool IsValidLogLevel(const std::string& logLevel)
{
    return logLevel == "trace" || logLevel == "debug" || logLevel == "info" || logLevel == "warn" || logLevel == "error"
           || logLevel == "critical" || logLevel == "off";
}

} // namespace

int main(int argc, char** argv)
{
    CommandlineParser commandlineParser;
    commandlineParser.SetDescription(
        "Experimental management client: watches the simulations on a registry with 'Experimental.Management.Enable'.");
    commandlineParser.Add<CommandlineParser::Flag>("help", "h", "-h, --help",
                                                   std::vector<std::string>{"Get this help."});
    commandlineParser.Add<CommandlineParser::Option>(
        "registry-uri", "u", "silkit://localhost:8500", "-u, --registry-uri <uri>",
        std::vector<std::string>{"The registry URI to connect to; a simulation path is ignored.",
                                 "Defaults to 'silkit://localhost:8500'."});
    commandlineParser.Add<CommandlineParser::Option>(
        "log", "l", "off", "-l, --log <level>",
        std::vector<std::string>{
            "Log to stdout with level:", "'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'.",
            "Defaults to 'off'."});
    commandlineParser.Add<CommandlineParser::Option>(
        "get-config", "g", "", "-g, --get-config <simulation>/<participant>",
        std::vector<std::string>{"Print the configuration the participant shared and exit.",
                                 "Use '/<participant>' for the default simulation."});

    try
    {
        commandlineParser.ParseArguments(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr);
        return -1;
    }

    if (commandlineParser.Get<CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout);
        return 0;
    }

    const auto registryUri = commandlineParser.Get<CommandlineParser::Option>("registry-uri").Value();

    auto logLevel = commandlineParser.Get<CommandlineParser::Option>("log").Value();
    if (!IsValidLogLevel(logLevel))
    {
        std::cerr << "Error: argument of '--log' must be one of 'trace', 'debug', 'warn', 'info', 'error', "
                     "'critical' or 'off'"
                  << std::endl;
        return -1;
    }
    logLevel[0] = static_cast<char>(std::toupper(logLevel[0]));

    const auto getConfig = commandlineParser.Get<CommandlineParser::Option>("get-config").Value();
    const auto separator = getConfig.rfind('/');
    if (!getConfig.empty() && (separator == std::string::npos || separator + 1 == getConfig.size()))
    {
        std::cerr << "Error: argument of '--get-config' must be '<simulation>/<participant>'" << std::endl;
        return -1;
    }

    try
    {
        const auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(
            R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":")" + logLevel + R"("}]}})");

        // declared before the client, which calls into it until the client is destroyed
        WatchState state;
        auto client = Mgmt::CreateManagementClient(participantConfiguration, registryUri);

        if (!getConfig.empty())
        {
            return PrintParticipantConfiguration(*client, getConfig.substr(0, separator),
                                                 getConfig.substr(separator + 1));
        }

        Watch(*client, state, registryUri);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
