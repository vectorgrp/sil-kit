// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/IParticipantController.hpp"
#include "ib/mw/sync/IIbToParticipantController.hpp"

#include <future>
#include <tuple>

#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/Config.hpp"

#include "PerformanceMonitor.hpp"

namespace ib {
namespace mw {
namespace sync {

class ParticipantController;

class TaskRunner
{
public:
    TaskRunner(ParticipantController& controller);
    
    void Run();
    void GrantReceived();

    virtual void RequestStep(ParticipantController& controller) = 0;
    virtual void FinishedStep(ParticipantController& controller) = 0;

private:
    ParticipantController & _controller;
};

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
    ParticipantController(IComAdapter* comAdapter, cfg::Participant participantConfig, cfg::TimeSync timesyncConfig);
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
    void SetSimulationTask(std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)> task) override;
    void SetSimulationTask(std::function<void(std::chrono::nanoseconds now)> task) override;

    void EnableColdswap() override;

    void SetPeriod(std::chrono::nanoseconds period) override;
    void SetEarliestEventTime(std::chrono::nanoseconds eventTime) override;

    auto Run() -> ParticipantState override;
    auto RunAsync() -> std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;
    void Pause(std::string reason) override;
    void Continue() override;
    void Stop(std::string reason) override;

    auto State() const -> ParticipantState override;
    auto Status() const -> const ParticipantStatus& override;
    void RefreshStatus() override;
    auto Now() const -> std::chrono::nanoseconds override;

    void LogCurrentPerformanceStats() override;

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
    void AdvanceQuantum();
    void ExecuteSimTask();

private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;

    void StartTaskRunner();

    void ChangeState(ParticipantState newState, std::string reason);
    
    void Initialize(const ParticipantCommand& command, std::string reason);
    void Shutdown(std::string reason);
    void PrepareColdswap();
    void ShutdownForColdswap();
    void IgnoreColdswap();
    void ProcessQuantumGrant(const QuantumGrant& msg);
    
private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    cfg::Participant _participantConfig;
    cfg::TimeSync _timesyncConfig;
    std::shared_ptr<spdlog::logger> _logger;

    std::unique_ptr<TaskRunner> _taskRunner;
    std::chrono::nanoseconds _period{0};
    bool _coldswapEnabled{false};

    ParticipantStatus _status;
    std::chrono::nanoseconds _now{0};
    std::chrono::nanoseconds _duration{0};

    std::promise<ParticipantState> _finalStatePromise;

    InitHandlerT _initHandler;
    StopHandlerT _stopHandler;
    ShutdownHandlerT _shutdownHandler;
    SimTaskT _simTask;
    std::future<void> _asyncResult;

    util::PerformanceMonitor _execTimeMonitor;
    util::PerformanceMonitor _waitTimeMonitor;
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
