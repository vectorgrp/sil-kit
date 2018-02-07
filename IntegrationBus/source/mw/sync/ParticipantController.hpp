// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/IParticipantController.hpp"
#include "ib/mw/sync/IIbToParticipantController.hpp"

#include <tuple>

#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/Config.hpp"


namespace ib {
namespace mw {
namespace sync {

class ITaskRunner;

class ParticipantController
    : public IParticipantController
    , public IIbToParticipantController
{
public:
    // ----------------------------------------
    // Public Data Types
    
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    ParticipantController() = default;
    ParticipantController(IComAdapter* comAdapter, cfg::Participant participantConfig);
    ParticipantController(const ParticipantController& other) = default;
    ParticipantController(ParticipantController&& other) = default;
    ParticipantController& operator=(const ParticipantController& other) = default;
    ParticipantController& operator=(ParticipantController&& other) = default;
    
public:
    // ----------------------------------------
    // Public Methods
    // IParticipantController
    void SetInitHandler(InitHandlerT handler) override;
    void SetStopHandler(StopHandlerT handler) override;
    void SetShutdownHandler(ShutdownHandlerT handler) override;
    void SetSimulationTask(SimTaskT task) override;

    void SetPeriod(std::chrono::nanoseconds period) override;
    void SetEarliestEventTime(std::chrono::nanoseconds eventTime) override;
    void EnableStrictSync() override;

    auto Run() -> ParticipantState override;
    auto RunAsync() -> std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;
    void Pause(std::string reason) override;
    void Continue() override;
    void Stop(std::string reason) override;

    auto State() const -> ParticipantState override;
    auto Status() const -> const ParticipantStatus& override;
    auto Now() const -> std::chrono::nanoseconds override;

    // IIbToParticipantController
    void SetEndpointAddress(const mw::EndpointAddress& addr) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;
    
    void ReceiveIbMessage(mw::EndpointAddress from, const ParticipantCommand& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const SystemCommand& msg) override;

    void ReceiveIbMessage(mw::EndpointAddress from, const Tick& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const QuantumGrant& msg) override;

    // Used by Policies
    void SendTickDone() const;
    void SendQuantumRequest() const;
    void ExecuteSimTask();

private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;

    template <template <class> class TaskRunnerT>
    void StartTaskRunner();

    void ChangeState(ParticipantState newState, std::string reason);
    
private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    cfg::Participant _participantConfig;

    std::unique_ptr<ITaskRunner> _taskRunner;
    bool _strictSync{false};
    std::chrono::nanoseconds _period{0};

    ParticipantStatus _status;
    std::chrono::nanoseconds _now{0};

    std::promise<ParticipantState> _finalStatePromise;

    InitHandlerT _initHandler;
    StopHandlerT _stopHandler;
    ShutdownHandlerT _shutdownHandler;
    SimTaskT _simTask;
};

class ITaskRunner
{
public:
    virtual ~ITaskRunner() = default;
    virtual void Start() = 0;
    virtual void Initialize() = 0;
    virtual void Run() = 0;
    virtual void GrantReceived() = 0;
    virtual void Stop() = 0;
    virtual void Shutdown() = 0;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void ParticipantController::SendIbMessage(MsgT&& msg) const
{
    _comAdapter->SendIbMessage(_endpointAddress, std::forward<MsgT>(msg));
}

    
} // namespace sync
} // namespace mw
} // namespace ib
