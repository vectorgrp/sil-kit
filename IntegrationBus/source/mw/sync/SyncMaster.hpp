// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <future>
#include <memory>
#include <unordered_map>
#include <vector>

#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ISyncMaster.hpp"
#include "ib/cfg/fwd_decl.hpp"

#include "IIbToSyncMaster.hpp"

namespace ib {
namespace mw {
namespace sync {


struct SyncRequest
{
public:
    SyncRequest() = default;
    SyncRequest(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) noexcept
        : status{Status::Pending}
        , now {now}
        , duration{duration}
    {}

    void Clear() noexcept
    {
        using namespace std::chrono_literals;
        status = SyncRequest::Status::Idle;
        now = 0ns;
        duration = 0ns;
    }

    void MarkGranted() noexcept
    {
        status = SyncRequest::Status::Granted;
    }
    void MarkIdle() noexcept
    {
        status = SyncRequest::Status::Idle;
    }

public:
    // --------------------
    // public members
    enum class Status
    {
        Idle,
        Pending,
        Granted
    };

    Status status{Status::Idle};
    std::chrono::nanoseconds now{0};
    std::chrono::nanoseconds duration{0};
};

class SyncClient
{
public:
    virtual ~SyncClient() = default;

    auto Now() const noexcept { return request.now; }
    auto Duration() const noexcept { return request.duration; }
    auto EndTime() const noexcept { return request.now + request.duration; }

    bool IsRunning() const noexcept
    {
        return request.status == SyncRequest::Status::Granted;
    }
    bool HasPendingRequest() const noexcept
    {
        return request.status == SyncRequest::Status::Pending;
    }
    void SetPendingRequest(std::chrono::nanoseconds now, std::chrono::nanoseconds quantum)
    {
        request = SyncRequest{now, quantum};
    }
    void GiveGrant()
    {
        request.MarkGranted();
        _grantAction(QuantumRequestStatus::Granted);
    }
    void RejectGrant()
    {
        request.MarkIdle();
        _grantAction(QuantumRequestStatus::Rejected);
    }

    void SetGrantAction(std::function<void(QuantumRequestStatus)> action)
    {
        _grantAction = std::move(action);
    }

    virtual void Reset()
    {
        request.Clear();
    }

protected:
    SyncRequest request;

private:
    std::function<void(QuantumRequestStatus)> _grantAction;
};

class TimeQuantumClient : public SyncClient
{
};

class DiscreteTimeClient : public SyncClient
{
public:
    DiscreteTimeClient(std::chrono::nanoseconds tickDuration)
        : tickDuration{tickDuration}
    {
        using namespace std::chrono_literals;
        request = SyncRequest{0ns, tickDuration};
    }

    void SetNumClients(unsigned int numClients_) noexcept { numClients = numClients_; }
    auto GetNumClients() const noexcept { return numClients; }

    void SetCurrentTick(Tick tick) noexcept { _currentTick = tick; }
    auto GetCurrentTick() const noexcept { return _currentTick; }

    void Reset() override
    {
        using namespace std::chrono_literals;
        request = SyncRequest{0ns, tickDuration};
        numTickDoneReceived = 0;
    }

    void TickDoneReceived()
    {
        numTickDoneReceived++;

        if (numClients == numTickDoneReceived)
        {
            request = SyncRequest{request.now + tickDuration, tickDuration};
            numTickDoneReceived = 0;
        }
    }

private:
    std::chrono::nanoseconds tickDuration{0};
    Tick _currentTick;

    unsigned int numClients{0};
    unsigned int numTickDoneReceived{0};
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
    void SendTick(const Tick& tick);
    void SendQuantumGrant(const QuantumGrant& grant);

private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress;
    logging::ILogger* _logger{nullptr};

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
