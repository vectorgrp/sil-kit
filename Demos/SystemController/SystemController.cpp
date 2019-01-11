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

std::ostream& operator<<(std::ostream& out, const ib::mw::sync::ParticipantStatus& status)
{
    std::time_t enterTime = std::chrono::system_clock::to_time_t(status.enterTime);
    char timebuffer[32];
    std::strftime(timebuffer, sizeof(timebuffer), "%F %T", std::localtime(&enterTime));

    out << timebuffer
        << " " << status.participantName
        << "\t State: " << status.state
        << "\t Reason: " << status.enterReason;
    
    return out;
}

void ReportParticipantStatus(const ib::mw::sync::ParticipantStatus& status)
{
    std::cout << status << std::endl;
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
    }

    void OnSystemStateChanged(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Idle:
            for (auto&& participant : ibConfig.simulationSetup.participants)
            {
                if (participant.syncType == cfg::SyncType::Unsynchronized)
                    continue;

                std::cout << "Sending ParticipantCommand::Init to participant \"" << participant.name << "\"" << std::endl;
                _controller->Initialize(participant.id);
            }
            return;
        case SystemState::Initialized:
            std::cout << "Sending SystemCommand::Run" << std::endl;
            _controller->Run();
            return;
        case SystemState::Stopped:
            std::cout << "Sending SystemCommand::Shutdown" << std::endl;
            _controller->Shutdown();
            return;
        case SystemState::Shutdown:
            _shutdownPromise.set_value(true);
            return;
        }
    }

    void Shutdown()
    {
        if (_monitor->SystemState() != SystemState::Running)
        {
            std::cerr << "IB is not Running. Terminating Process without Stopping." << std::endl;
            std::cout << "Sending SystemCommand::Shutdown" << std::endl;
            _controller->Shutdown();
            std::this_thread::sleep_for(1s);
            return;
        }
        std::cout << "Sending SystemCommand::Stop" << std::endl;
        _controller->Stop();
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

    // Create a SyncMaster if the participant is configured to do so.
    // NB: New SyncMaster Interface is subject to changes (cf. AFTMAGT-124)
    auto& participantConfig = ib::cfg::get_by_name(ibConfig.simulationSetup.participants, participantName);
    if (participantConfig.isSyncMaster)
    {
        comAdapter->CreateSyncMaster();
        std::cout << "Created SyncMaster at Participant: " << participantName << std::endl;
    }


    IbController ibController(comAdapter.get(), ibConfig);

    std::cout << "Press enter to Shutdown the Integration Bus..." << std::endl;
    std::cin.ignore();

    ibController.Shutdown();

    return 0;
}
