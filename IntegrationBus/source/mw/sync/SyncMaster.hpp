// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ISyncMaster.hpp"
#include "ib/mw/sync/IIbToSyncMaster.hpp"
#include "ib/cfg/fwd_decl.hpp"

namespace ib {
namespace mw {
namespace sync {


struct SyncClient
{
    std::chrono::nanoseconds now{0};
    std::chrono::nanoseconds endTime{0};

    enum class RequestState
    {
        Idle,
        Pending,
        Granted
    };
    RequestState request;

    virtual ~SyncClient() = default;

    bool IsRunning() const
    {
        return request == RequestState::Granted;
    }
    bool HasPendingRequest() const
    {
        return request == RequestState::Pending;
    }
    void SetPendingRequest(std::chrono::nanoseconds now_, std::chrono::nanoseconds quantum)
    {
        now = now_;
        endTime = now_ + quantum;
        request = RequestState::Pending;
    }
    void GiveGrant()
    {
        request = RequestState::Granted;
        _grantAction(QuantumRequestStatus::Granted);
    }
    void RejectGrant()
    {
        request = RequestState::Idle;
        _grantAction(QuantumRequestStatus::Rejected);
    }

    void SetGrantAction(std::function<void(QuantumRequestStatus)> action)
    {
        _grantAction = std::move(action);
    }

private:
    std::function<void(QuantumRequestStatus)> _grantAction;
};

struct TimeQuantumClient : SyncClient
{
};

struct DiscreteTimeClient : SyncClient
{
    std::chrono::nanoseconds tickDuration{0};

    unsigned int numClients{0};
    unsigned int numTickDoneReceived{0};

    void TickDoneReceived()
    {
        numTickDoneReceived++;

        if (numClients == numTickDoneReceived)
        {
            SetPendingRequest(endTime, tickDuration);
            numTickDoneReceived = 0;
        }
    }

};



class SyncMaster :
    public ISyncMaster,
    public IIbToSyncMaster
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    SyncMaster() = default;
    SyncMaster(const SyncMaster&) = default;
    SyncMaster(SyncMaster&&) = default;
    SyncMaster(IComAdapter* comAdapter, const cfg::Config& config, ISystemMonitor* monitor);


public:
    // ----------------------------------------
    // Operator Implementations
    SyncMaster& operator=(SyncMaster& other) = default;
    SyncMaster& operator=(SyncMaster&& other) = default;

public:
    // ----------------------------------------
    // Public Methods
    void ReceiveIbMessage(mw::EndpointAddress from, const TickDone& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const QuantumRequest& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    void WaitForShutdown() override;

private:
    // ----------------------------------------
    // private data types


private:
    // ----------------------------------------
    // private methods
    void SetupTimeQuantumClients(const cfg::Config& config);
    void SetupDiscreteTimeClient(const cfg::Config& config);

    void SystemStateChanged(SystemState newState);

    void SendGrants();
    void SendTick(std::chrono::nanoseconds now);
    void SendQuantumGrant(const QuantumGrant& grant);

private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress;

    SystemState _systemState{SystemState::Invalid};
    std::promise<SystemState> _finalStatePromise;

    // The combined list of all SyncClients, i.e., TimeQuantum + DiscreteTime
    std::vector<std::shared_ptr<SyncClient>> _syncClients;

    // SyncClients, which represent individual participants with SyncType::TimeQuantum
    std::unordered_map<mw::ParticipantId, std::shared_ptr<TimeQuantumClient>> _timeQuantumClients;

    // SyncClient, which represents all participants with SyncType::DiscreteTime
    std::shared_ptr<DiscreteTimeClient> _discreteTimeClient;
    std::chrono::nanoseconds _maxGrantedEndTime{0};
};

// ================================================================================
//  Inline Implementations
// ================================================================================



} // namespace sync
} // namespace mw
} // namespace ib
