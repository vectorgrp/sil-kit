/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <future>
#include <tuple>
#include <map>

#include "silkit/services/orchestration/ILifecycleService.hpp"

#include "PerformanceMonitor.hpp"
#include "WatchDog.hpp"

#include "IMsgForLifecycleService.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//forward declarations
class SynchronizedVirtualTimeProvider;
class TimeSyncService;
class ILifecycleManagement;
struct LifecycleConfiguration;

class ILifecycleServiceInternal
    : public ILifecycleServiceWithTimeSync
    , public ILifecycleServiceNoTimeSync
{
public:
    ~ILifecycleServiceInternal() = default;

public:
    // ILifecycleServiceWithTimeSync and ILifecycleServiceNoTimeSync
    virtual void SetCommunicationReadyHandler(CommunicationReadyHandler handler) = 0;
    virtual void SetStopHandler(StopHandler handler) = 0;
    virtual void SetShutdownHandler(ShutdownHandler handler) = 0;
    virtual auto StartLifecycle(LifecycleConfiguration startConfiguration) -> std::future<ParticipantState> = 0;
    virtual void ReportError(std::string errorMsg) = 0;
    virtual void Pause(std::string reason) = 0;
    virtual void Continue() = 0;
    virtual void Stop(std::string reason) = 0;
    virtual auto State() const -> ParticipantState = 0;
    virtual auto Status() const -> const ParticipantStatus& = 0;

    // ILifecycleServiceNoTimeSync
    virtual void SetStartingHandler(StartingHandler handler) = 0;

    // ILifecycleServiceWithTimeSync
    virtual auto GetTimeSyncService() const -> ITimeSyncService* = 0;

    // internal only
    virtual void SetTimeSyncActive(bool isTimeSyncActive) = 0;
};

class LifecycleService
    : public ILifecycleServiceInternal
    , public IMsgForLifecycleService
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    LifecycleService(Core::IParticipantInternal* participant,
                     const Config::HealthCheck& healthCheckConfig);

public:
    // ----------------------------------------
    // Public Methods
    // ILifecycleService
    void SetCommunicationReadyHandler(CommunicationReadyHandler handler) override;
    void SetStartingHandler(StartingHandler handler) override;
    void SetStopHandler(StopHandler handler) override;
    void SetShutdownHandler(ShutdownHandler handler) override;

    auto GetTimeSyncService() const -> ITimeSyncService* override;

    auto StartLifecycle(LifecycleConfiguration startConfiguration)
        -> std::future<ParticipantState> override;

    void ReportError(std::string errorMsg) override;

    void Pause(std::string reason) override;
    void Continue() override;

    void Stop(std::string reason) override;

    auto State() const -> ParticipantState override;
    auto Status() const -> const ParticipantStatus& override;

    void ReceiveMsg(const IServiceEndpoint* from, const ParticipantCommand& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const SystemCommand& msg) override;

    void SetTimeSyncActive(bool isTimeSyncActive) override;
    // Used by Policies
    template <class MsgT>
    void SendMsg(MsgT&& msg) const;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;


public:
    void TriggerCommunicationReadyHandler(std::string reason);
    void TriggerStartingHandler(std::string reason);
    void TriggerStopHandler(std::string reason);
    void TriggerShutdownHandler(std::string reason);

    void ChangeState(ParticipantState newState, std::string reason);

    void SetTimeSyncService(TimeSyncService* timeSyncService);

    void NewSystemState(SystemState systemState);

    bool IsTimeSyncActive() const;


private:
    // ----------------------------------------
    // private methods
    auto StartLifecycle(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
        -> std::future<ParticipantState>;

    void Shutdown(std::string reason);
    void Restart(std::string reason);
    void AbortSimulation(std::string reason);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Logging::ILogger* _logger{nullptr};

    TimeSyncService* _timeSyncService;

    bool _hasCoordinatedSimulationStart = false;
    bool _hasCoordinatedSimulationStop = false;

    bool _isRunning{false};
    ParticipantStatus _status;
    std::shared_ptr<ILifecycleManagement> _lifecycleManagement;
    bool _timeSyncActive = false;

    std::promise<ParticipantState> _finalStatePromise;

    CommunicationReadyHandler _commReadyHandler;
    StartingHandler _startingHandler;
    StopHandler _stopHandler;
    ShutdownHandler _shutdownHandler;
    std::future<void> _asyncResult;

    // When pausing our participant, message processing is deferred
    // until Continue()  is called;
    std::promise<void> _pauseDonePromise;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
void LifecycleService::SendMsg(MsgT&& msg) const
{
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}

void LifecycleService::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LifecycleService::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
