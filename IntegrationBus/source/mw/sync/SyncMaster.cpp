// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SyncMaster.hpp"

#include <algorithm>
#include <cassert>

#include "ib/cfg/Config.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "SyncDatatypeUtils.hpp"

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

SyncMaster::SyncMaster(IComAdapterInternal* comAdapter, const cfg::Config& config, ISystemMonitor* monitor)
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
        auto& participantController = participant.participantController;
        if (!participantController)
            continue;

        if (participantController->syncType == cfg::SyncType::Unsynchronized)
        {
            _logger->Error("Participant {} uses a ParticipantController, which has not configured a SyncType!", participant.name);
            continue;
        }

        if (participantController->syncType != cfg::SyncType::TimeQuantum)
            continue;

        auto client = std::make_shared<TimeQuantumClient>();
        std::weak_ptr<TimeQuantumClient> clientWeakPtr(client);
        client->SetGrantAction(
            [this, clientWeakPtr, participantId = participant.id](QuantumRequestStatus status)
            {
                if (auto clientSharedPtr = clientWeakPtr.lock())
                {
                    QuantumGrant grant;
                    grant.grantee = mw::EndpointAddress{ participantId, 1024 };
                    grant.now = clientSharedPtr->Now();
                    grant.duration = clientSharedPtr->Duration();
                    grant.status = status;

                    this->SendQuantumGrant(grant);
                }
            });

        _syncClients.push_back(client);
        _timeQuantumClients[participant.id] = std::move(client);

    }
    _logger->Info("SyncMaster is serving {} TimeQuantum Clients", _timeQuantumClients.size());
}

void SyncMaster::SetupDiscreteTimeClient(const cfg::Config& config)
{
    auto numClients = std::count_if(
        begin(config.simulationSetup.participants),
        end(config.simulationSetup.participants),
        [](auto&& participant) {
            return participant.participantController.has_value()
                && (participant.participantController->syncType == cfg::SyncType::DiscreteTime);
        }
    );
    _logger->Info("SyncMaster is serving {} DiscreteTime Clients", numClients);

    if (numClients == 0)
        return;

    auto client = std::make_shared<DiscreteTimeClient>(config.simulationSetup.timeSync.tickPeriod);
    client->SetNumClients(static_cast<unsigned int>(numClients));

    client->SetCurrentTick(Tick{0ns, 0ns});

    std::weak_ptr<DiscreteTimeClient> clientWeakPtr(client);
    client->SetGrantAction(
        [this, clientWeakPtr](QuantumRequestStatus status)
        {
            auto clientSharedPtr = clientWeakPtr.lock();
            if (status == QuantumRequestStatus::Granted && clientSharedPtr)
            {
                Tick tick{ clientSharedPtr->Now(), clientSharedPtr->Duration()};
                clientSharedPtr->SetCurrentTick(tick);
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
        _logger->Error("Received {} from participant {}, which does not match current {}", msg, from.participant, _discreteTimeClient->GetCurrentTick());
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
        _logger->Error("Received QuantumRequest from participant {}, which is unknown!", from.participant);
        return;
    }

    auto&& client = _timeQuantumClients.at(from.participant);

    if (client->HasPendingRequest())
    {
        _logger->Error("Received QuantumRequest from participant {}, which already has a pending request!", from.participant);
        return;
    }

    if (client->EndTime() != msg.now)
    {
        _logger->Error("QuantumRequest from participant {} does not match the current simulation time!", from.participant);
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
    _logger->Info("New SystemState::{}", newState);
    auto oldState = _systemState;
    _systemState = newState;

    if (newState == SystemState::Running)
    {
        switch (oldState)
        {
        case SystemState::Paused:
            _logger->Info("SyncMaster: continuing simulating.");
            break;

        case SystemState::Initializing:
            // There is a chance to go directly from Initializing to Running due
            // to incoherent participant state transitions. We consider this also
            // a start of simulation.
            //[[fallthrough]]

        case SystemState::Initialized:
            _logger->Info("SyncMaster: starting simulating.");
            for (auto&& client : _syncClients)
            {
                client->Reset();
            }
            break;

        default:
            _logger->Warn("SyncMaster: switch to SystemState::Running from unexpected state SystemState::{}. Assuming simulation start.", oldState);
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
    auto minClientNowIterator =
        std::min_element(_syncClients.begin(), _syncClients.end(), [](const auto& a, const auto& b)
        {
            return a->Now() < b->Now();
        });
    if (minClientNowIterator == _syncClients.end())
    {
        // _syncClients is empty. No Grants to send
        return;
    }
    auto minNow = (*minClientNowIterator)->Now();

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
