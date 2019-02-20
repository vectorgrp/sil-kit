// Copyright (c)  Vector Informatik GmbH. All rights reserved.

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
#include "ib/mw/sync/string_utils.hpp"


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
    IbController(IComAdapter* comAdapter, cfg::Config ibConfig)
        : ibConfig{std::move(ibConfig)}
    {
        _controller = comAdapter->GetSystemController();
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
        }
    }

    void InitializeAllParticipants()
    {
        for (auto&& participant : ibConfig.simulationSetup.participants)
        {
            if (participant.syncType == cfg::SyncType::Unsynchronized)
                continue;

            std::cout << "Sending ParticipantCommand::Init to participant \"" << participant.name << "\"" << std::endl;
            _controller->Initialize(participant.id);
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

public:
    // ----------------------------------------
    //  Public Members
    ib::cfg::Config ibConfig;
    bool _stopInitiated{false};
    bool _performRestart{false};
    std::promise<bool> _shutdownPromise;

    ISystemController* _controller;
    ISystemMonitor* _monitor;

};

int main(int argc, char** argv)
{
    ib::cfg::Config ibConfig;
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }
    
    try
    {
        auto jsonFilename = std::string(argv[1]);
        ibConfig = ib::cfg::Config::FromJsonFile(jsonFilename);
    }
    catch (ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << (&error)->what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    std::string participantName{"SystemController"};

    uint32_t domainId = 42;
    if (argc >= 3)
    {
        try
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[2]));
        }
        catch (std::exception&)
        {
        }
    }

    std::cout << "Creating SystemController for IB domain=" << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);

    IbController ibController(comAdapter.get(), ibConfig);

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

    return 0;
}
