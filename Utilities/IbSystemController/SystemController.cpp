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

#include "CommandlineParser.hpp"

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

class IbController
{
public:
    IbController(ib::mw::IParticipant* participant, std::shared_ptr<ib::cfg::IParticipantConfiguration> ibConfig,
                 const std::vector<std::string>& expectedParticipantNames)
        : _ibConfig{std::move(ibConfig)}
        , _expectedParticipantNames{expectedParticipantNames}
    {
        _controller = participant->GetSystemController();
        _controller->SetWorkflowConfiguration({expectedParticipantNames});

        _monitor = participant->GetSystemMonitor();
        _monitor->AddSystemStateHandler([this](SystemState newState) {
            this->OnSystemStateChanged(newState);
        });
        _monitor->AddParticipantStatusHandler([this](const ParticipantStatus& newStatus) {
            this->OnParticipantStatusChanged(newStatus);
        });
    }

    void OnSystemStateChanged(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::ReadyToRun:
            std::cout << "Sending SystemCommand::Run" << std::endl;
            _controller->Run();
            return;
        case SystemState::Stopping:
            return;

        case SystemState::Stopped:
            std::cout << "Sending ParticipantCommand::Shutdown" << std::endl;
            for (auto&& name: _expectedParticipantNames)
            {
                _controller->Shutdown(name);
            }
            return;

        case SystemState::Shutdown:
            _shutdownPromise.set_value(true);
            return;
        default:
            //not handled
            break;
        }
    }

    void OnParticipantStatusChanged(const ParticipantStatus& newStatus)
    {
        switch (newStatus.state)
        {
        case ParticipantState::Stopped:
            if (!_stopInitiated)
            {
                // We did not initiate this Stop, so some Participant must have called ParticipantClient::Stop().
                // --> Propagate the Stop to all participants.
                std::cout << "Detected voluntary stop by participant " << newStatus.participantName << std::endl;
                Stop();
            }
            break;
        default:
            // not handled
            break;
        }
    }

    void Stop()
    {
        std::cout << "Sending SystemCommand::Stop" << std::endl;
        _stopInitiated = true;
        _controller->Stop();
    }

    void Shutdown()
    {
        if (_monitor->SystemState() == SystemState::Running)
        {
            Stop();
        }
        else
        {
            std::cerr << "IB is not Running. Terminating Process without Stopping." << std::endl;
            std::cout << "Sending SystemCommand::Shutdown" << std::endl;
            for (auto&& name: _expectedParticipantNames)
            {
                _controller->Shutdown(name);
            }
            std::this_thread::sleep_for(1s);
            return;
        }

        auto future = _shutdownPromise.get_future();
        auto status = future.wait_for(5s);
        if (status != std::future_status::ready)
        {
            std::cerr << "IB did not shut down in 5s... Terminating Process." << std::endl;
            std::this_thread::sleep_for(1s);
            return;
        }
    }

private:
    std::shared_ptr<ib::cfg::IParticipantConfiguration> _ibConfig;
    std::vector<std::string> _expectedParticipantNames;
    bool _stopInitiated{false};
    std::promise<bool> _shutdownPromise;

    ISystemController* _controller;
    ISystemMonitor* _monitor;
};

int main(int argc, char** argv)
{
    using namespace ib::util;
    CommandlineParser commandlineParser;
    commandlineParser.Add<CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<CommandlineParser::Option>("domain", "d", "42", "[--domain <domainId>]",
        "-d, --domain <domainId>: The domain ID that is used by the Integration Bus. Defaults to 42.");
    commandlineParser.Add<CommandlineParser::Option>(
        "connect-uri", "u", "vib://localhost:8500", "[--connect-uri <vibUri>]",
        "-u, --connect-uri <vibUri>: The registry URI to connect to. Defaults to vib://localhost:8500.");
    commandlineParser.Add<CommandlineParser::Option>("name", "n", "SystemController", "[--name <participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'SystemController'.");
    commandlineParser.Add<CommandlineParser::Option>("configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. Note that the format was changed in v3.6.11.");
    commandlineParser.Add<CommandlineParser::PositionalList>("participantNames", "<participantName1> [<participantName2> ...]",
        "<participantName1>, <participantName2>, ...: Names of participants to wait for before starting simulation.");

    std::cout 
        << "Vector Integration Bus (VIB) -- System Controller\n"
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

    if (commandlineParser.Get<CommandlineParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);

        return 0;
    }

    if (commandlineParser.Get<CommandlineParser::Flag>("version").Value())
    {
        std::string ibHash{ ib::version::GitHash() };
        auto ibShortHash = ibHash.substr(0, 7);
        std::cout
            << "Version Info:" << std::endl
            << " - Vector Integration Bus (VIB): " << ib::version::String() << ", #" << ibShortHash << std::endl;

        return 0;
    }

    if (!commandlineParser.Get<CommandlineParser::PositionalList>("participantNames").HasValues())
    {
        std::cerr << "Error: Arguments '<participantName1> [<participantName2> ...]' are missing" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    auto domain{ commandlineParser.Get<CommandlineParser::Option>("domain").Value() };
    auto participantName{ commandlineParser.Get<CommandlineParser::Option>("name").Value() };
    auto configurationFilename{ commandlineParser.Get<CommandlineParser::Option>("configuration").Value() };
    auto expectedParticipantNames{
        commandlineParser.Get<CommandlineParser::PositionalList>("participantNames").Values()};
    auto connectUri{ commandlineParser.Get<CommandlineParser::Option>("connect-uri").Value() };

    uint32_t domainId;
    try
    {
        domainId = static_cast<uint32_t>(std::stoul(domain));
    }
    catch (const std::exception&)
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
    catch (const ib::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -2;
    }

    try
    {
        std::cout
            << "Creating participant '" << participantName
            << "' at domain " << domainId << ", expecting participant"
            << (expectedParticipantNames.size() > 1 ? "s '" : " '");
        std::copy(expectedParticipantNames.begin(), std::prev(expectedParticipantNames.end()),
                  std::ostream_iterator<std::string>(std::cout, "', '"));
        std::cout << expectedParticipantNames.back() << "'..." << std::endl;

        auto participant = ib::CreateParticipant(configuration, participantName, connectUri);

        IbController ibController(participant.get(), configuration, expectedParticipantNames);

        std::cout << "Press enter to shutdown the Integration Bus..." << std::endl;
        std::cin.ignore();

        ibController.Shutdown();
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
