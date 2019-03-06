// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SyncMaster.hpp"

#include <algorithm>
#include <cassert>

#include "ib/cfg/Config.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/spdlog.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "SyncDatatypeUtils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

SyncMaster::SyncMaster(IComAdapter* comAdapter, const cfg::Config& config, ISystemMonitor* monitor)
    : _comAdapter{comAdapter}
    , _logger{comAdapter->GetLogger()}
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
    _logger->info("SyncMaster is serving {} TimeQuantum Clients", _timeQuantumClients.size());
}

void SyncMaster::SetupDiscreteTimeClient(const cfg::Config& config)
{
    auto numClients = std::count_if(
        begin(config.simulationSetup.participants),
        end(config.simulationSetup.participants),
        [](auto&& participant) { return participant.syncType == cfg::SyncType::DiscreteTime; }
    );
    _logger->info("SyncMaster is serving {} DiscreteTime Clients", numClients);

    if (numClients == 0)
        return;

    auto client = std::make_shared<DiscreteTimeClient>(config.simulationSetup.timeSync.tickPeriod);
    client->SetNumClients(static_cast<unsigned int>(numClients));

    client->SetCurrentTick(Tick{0ns, 0ns});
    client->SetGrantAction(
        [this, client](QuantumRequestStatus status)
        {
            if (status == QuantumRequestStatus::Granted)
            {
                Tick tick{client->Now(), client->Duration()};
                client->SetCurrentTick(tick);
                this->SendTick(tick);
            }
        });

    _syncClients.push_back(client);
    _discreteTimeClient = std::move(client);
}

void SyncMaster::ReceiveIbMessage(mw::EndpointAddress from, const TickDone& msg)
{
    assert(_discreteTimeClient);

    if (_discreteTimeClient->GetCurrentTick() != msg.finishedTick)
    {
        _logger->error("Received {} from participant {}, which does not match current {}", msg, from.participant, _discreteTimeClient->GetCurrentTick());
    }

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
        _logger->error("Received QuantumRequest from participant {}, which is unknown!", from.participant);
        return;
    }

    auto&& client = _timeQuantumClients.at(from.participant);

    if (client->HasPendingRequest())
    {
        _logger->error("Received QuantumRequest from participant {}, which already has a pending request!", from.participant);
        return;
    }

    if (client->EndTime() != msg.now)
    {
        _logger->error("QuantumRequest from participant {} does not match the current simulation time!", from.participant);
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
            _logger->info("SyncMaster: continuing simulating.");
            break;

        case SystemState::Initializing:
            // There is a chance to go directly from Initializing to Running due
            // to incoherent participant state transitions. We consider this also
            // a start of simulation.
            //[[fallthrough]]

        case SystemState::Initialized:
            _logger->info("SyncMaster: starting simulating.");
            for (auto&& client : _syncClients)
            {
                client->Reset();
            }
            break;

        default:
            _logger->warn("SyncMaster: switch to SystemState::Running from unexpected state SystemState::{}. Assuming simulation start.", oldState);
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


void SyncMaster::SendTick(const Tick& tick)
{
    _comAdapter->SendIbMessage(_endpointAddress, tick);
}

void SyncMaster::SendQuantumGrant(const QuantumGrant& grant)
{
    _comAdapter->SendIbMessage(_endpointAddress, grant);
}


} // namespace sync
} // namespace mw
} // namespace ib
