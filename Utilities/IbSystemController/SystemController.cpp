// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS 1

#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"

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
    IbController(ib::mw::IComAdapter* comAdapter, std::shared_ptr<ib::cfg::IParticipantConfiguration> ibConfig,
                 const std::vector<std::string>& expectedParticipantNames)
        : _ibConfig{std::move(ibConfig)}
        , _expectedParticipantNames{expectedParticipantNames}
    {
        _controller = comAdapter->GetSystemController();
        _controller->SetRequiredParticipants(expectedParticipantNames);

        _monitor = comAdapter->GetSystemMonitor();
        _monitor->RegisterSystemStateHandler(
            [this](SystemState newState) {
                this->OnSystemStateChanged(newState);
            }
        );
        _monitor->RegisterParticipantStatusHandler(
            [this](const ParticipantStatus& newStatus) {
                this->OnParticipantStatusChanged(newStatus);
            }
        );
    }

    void OnSystemStateChanged(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Idle:
            InitializeAllParticipants();
            return;
        case SystemState::Initialized:
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
        for (auto&& name : _expectedParticipantNames)
        {
            std::cout << "Sending ParticipantCommand::Init to participant \"" << name << "\"" << std::endl;
            _controller->Initialize(name);
        }
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
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <domainId> <participantName1> [participantName2] ..." << std::endl;
        return -1;
    }
    
    try
    {
        std::string participantName{"SystemController"};

        uint32_t domainId = static_cast<uint32_t>(std::stoul(argv[1]));

        std::vector<std::string> expectedParticipantNames;
        for (int i = 2; i < argc; i++)
        {
            expectedParticipantNames.push_back(argv[i]);
        }

        // TODO: Use config file from (optional) command-line argument
        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromString("{}");

        std::cout << "Creating SystemController for IB domain=" << domainId << std::endl;
        auto comAdapter = ib::CreateSimulationParticipant(participantConfiguration, participantName, domainId, false);
        
        IbController ibController(comAdapter.get(), participantConfiguration, expectedParticipantNames);

        // Set numRestarts to values larger than zero to test the restart functionality.
        int numRestarts = 0;
        for (int i = numRestarts; i > 0; i--)
        {
            std::cout << "Press enter to restart the Integration Bus..." << std::endl;
            std::cin.ignore();

            ibController.StopAndRestart();
        }

        std::cout << "Press enter to Shutdown the Integration Bus..." << std::endl;
        std::cin.ignore();

        ibController.Shutdown();
    }
    catch (const ib::configuration_error& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
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
