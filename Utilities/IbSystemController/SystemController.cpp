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
        _controller->SetRequiredParticipants(expectedParticipantNames);

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
        case SystemState::ServicesCreated:
            InitializeAllParticipants();
            return;
        case SystemState::ReadyToRun:
            std::cout << "Sending SystemCommand::Run" << std::endl;
            _controller->Run();
            return;
        case SystemState::Stopping:
            return;

        case SystemState::Stopped:
            if (_performRestart)
            {
                _performRestart = false;

                std::cout << "Restarting in ";
                for (int i = 3; i > 0; i--)
                {
                    std::cout << i << "... ";
                    std::this_thread::sleep_for(1s);
                }
                std::cout << std::endl;
                
                InitializeAllParticipants();
            }
            else
            {
                std::cout << "Sending SystemCommand::Shutdown" << std::endl;
                _controller->Shutdown();
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
        //case ParticipantState::Stopping:
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

    void InitializeAllParticipants()
    {
        std::cout << "All required participants have finished creating their controllers." << std::endl;
    }

    void Stop()
    {
        std::cout << "Sending SystemCommand::Stop" << std::endl;
        _stopInitiated = true;
        _controller->Stop();
    }

    void StopAndRestart()
    {
        _performRestart = true;
        Stop();
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
            _controller->Shutdown();
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
    bool _performRestart{false};
    std::promise<bool> _shutdownPromise;

    ISystemController* _controller;
    ISystemMonitor* _monitor;

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

    std::cout << "Vector Integration Bus (VIB) -- System Controller" << std::endl
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

        IbController ibController(participant.get(), configuration, expectedParticipantNames);

        // Set numRestarts to values larger than zero to test the restart functionality.
        int numRestarts = 0;
        for (int i = numRestarts; i > 0; i--)
        {
            std::cout << "Press enter to restart the Integration Bus..." << std::endl;
            std::cin.ignore();

            ibController.StopAndRestart();
        }

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
