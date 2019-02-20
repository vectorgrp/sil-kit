// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SyncMaster.hpp"

#include <algorithm>
#include <cassert>

#include "ib/cfg/Config.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

SyncMaster::SyncMaster(IComAdapter* comAdapter, const cfg::Config& config, ISystemMonitor* monitor)
    : _comAdapter{comAdapter}
{
    assert(monitor);
    monitor->RegisterSystemStateHandler([this](auto state) { this->SystemStateChanged(state); });

    SetupTimeQuantumClients(config);
    SetupDiscreteTimeClient(config);
}

void SyncMaster::SetupTimeQuantumClients(const cfg::Config& config)
{
    for (auto&& participant : config.simulationSetup.participants)
    {
        if (participant.syncType != cfg::SyncType::TimeQuantum)
            continue;

        auto client = std::make_shared<TimeQuantumClient>();
        client->SetGrantAction(
            [this, client, participantId = participant.id](QuantumRequestStatus status)
            {
                QuantumGrant grant;
                grant.grantee = mw::EndpointAddress{participantId, 1024};
                grant.now = client->Now();
                grant.duration = client->Duration();
                grant.status = status;

                this->SendQuantumGrant(grant);
            });

        _syncClients.push_back(client);
        _timeQuantumClients[participant.id] = std::move(client);

    }
    std::cout << "INFO: SyncMaster has " << _timeQuantumClients.size() << " TimeQuantum Clients\n";
}

void SyncMaster::SetupDiscreteTimeClient(const cfg::Config& config)
{
    auto numClients = std::count_if(
        begin(config.simulationSetup.participants),
        end(config.simulationSetup.participants),
        [](auto&& participant) { return participant.syncType == cfg::SyncType::DiscreteTime; }
    );
    std::cout << "INFO: SyncMaster has " << numClients << " DiscreteTime Clients\n";

    if (numClients == 0)
        return;

    auto client = std::make_shared<DiscreteTimeClient>(config.simulationSetup.timeSync.tickPeriod);
    client->SetNumClients(static_cast<unsigned int>(numClients));

    client->SetGrantAction(
        [this, client](QuantumRequestStatus status)
        {
            if (status == QuantumRequestStatus::Granted)
                this->SendTick(client->Now());
        });

    _syncClients.push_back(client);
    _discreteTimeClient = std::move(client);
}

void SyncMaster::ReceiveIbMessage(mw::EndpointAddress /*from*/, const TickDone& msg)
{
    assert(_discreteTimeClient);

    _discreteTimeClient->TickDoneReceived();

    // check if it was the last missing TickDone in this step
    if (!_discreteTimeClient->HasPendingRequest())
        return;

    if (_systemState == SystemState::Running)
    {
        SendGrants();
    }
}

void SyncMaster::ReceiveIbMessage(mw::EndpointAddress from, const QuantumRequest& msg)
{
    if (_timeQuantumClients.count(from.participant) != 1)
    {
        std::cerr << "ERROR: Received QuantumRequest from participant " << from.participant << ", which is unknown!\n";
        return;
    }

    auto&& client = _timeQuantumClients.at(from.participant);

    if (client->HasPendingRequest())
    {
        std::cerr << "ERROR: Received QuantumRequest from participant " << from.participant << ", which already has a pending request!\n";
        return;
    }

    if (client->EndTime() != msg.now)
    {
        std::cerr << "ERROR: QuantumRequest from participant " << from.participant << " does not match the current simulation time!\n";
    }

    client->SetPendingRequest(msg.now, msg.duration);

    if (_systemState == SystemState::Running)
    {
        SendGrants();
    }
}

void SyncMaster::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddress = endpointAddress;
}

auto SyncMaster::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddress;
}

void SyncMaster::WaitForShutdown()
{
    auto future = _finalStatePromise.get_future();
    future.wait();
    return;
}
    
void SyncMaster::SystemStateChanged(SystemState newState)
{
    auto oldState = _systemState;
    _systemState = newState;

    if (newState == SystemState::Running)
    {
        switch (oldState)
        {
        case SystemState::Paused:
            std::cerr << "INFO: SyncMaster: continuing simulating." << std::endl;
            break;

        case SystemState::Initializing:
            // There is a chance to go directly from Initializing to Running due
            // to incoherent participant state transitions. We consider this also
            // a start of simulation.
            //[[fallthrough]]

        case SystemState::Initialized:
            std::cerr << "INFO: SyncMaster: starting simulating." << std::endl;
            for (auto&& client : _syncClients)
            {
                client->Reset();
            }
            break;

        default:
            std::cerr << "WARNING: SyncMaster: switch to SystemState::Running from unexpected state SystemState::" << oldState << ". Assuming simulation start." << std::endl;
            for (auto&& client : _syncClients)
            {
                client->Reset();
            }
        }

        SendGrants();
    }
}

void SyncMaster::SendGrants()
{
    auto&& minClientNow =
        *std::min_element(_syncClients.begin(), _syncClients.end(), [](const auto& a, const auto& b)
        {
            return a->Now() < b->Now();
        });

    auto minNow = minClientNow->Now();

    for (auto&& client : _syncClients)
    {
        if (!client->HasPendingRequest())
            continue;

        if (client->Now() == minNow || client->EndTime() <= _maxGrantedEndTime)
        {
            client->GiveGrant();
            _maxGrantedEndTime = std::max(_maxGrantedEndTime, client->EndTime());
        }
    }

}


void SyncMaster::SendTick(std::chrono::nanoseconds now)
{
    Tick tick;
    tick.now = now;
    _comAdapter->SendIbMessage(_endpointAddress, tick);
}

void SyncMaster::SendQuantumGrant(const QuantumGrant& grant)
{
    _comAdapter->SendIbMessage(_endpointAddress, grant);
}


} // namespace sync
} // namespace mw
} // namespace ib
