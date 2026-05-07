// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>

#include "LifecycleManagement.hpp"
#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"
#include "procs/IParticipantReplies.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

LifecycleManagement::LifecycleManagement(Core::IParticipantInternal* participant, Services::Logging::ILogger* logger,
                                         LifecycleService* parentService)
    : _participant{participant}
    , _lifecycleService(parentService)
    , _logger(logger)
    , _stateMachine{*this}
{
}

// ILifecycleManagement
void LifecycleManagement::Initialize(std::string reason)
{
    _stateMachine.Initialize(std::move(reason));
}

void LifecycleManagement::ServicesCreated(std::string reason)
{
    _stateMachine.ServicesCreated(std::move(reason));
}

void LifecycleManagement::CommunicationInitialized(std::string reason)
{
    _stateMachine.CommunicationInitialized(std::move(reason));
}

void LifecycleManagement::CompleteCommunicationReadyHandler(std::string reason)
{
    _stateMachine.CompleteCommunicationReadyHandler(std::move(reason));
}

void LifecycleManagement::ReadyToRun(std::string reason)
{
    _stateMachine.ReadyToRun(std::move(reason));
}

void LifecycleManagement::Restart(std::string reason)
{
    _stateMachine.Restart(std::move(reason));
}

void LifecycleManagement::Shutdown(std::string reason)
{
    _stateMachine.Shutdown(std::move(reason));
}

void LifecycleManagement::NotifyShutdownInConnection()
{
    _participant->NotifyShutdown();
}

void LifecycleManagement::Pause(std::string reason)
{
    _stateMachine.Pause(std::move(reason));
}

void LifecycleManagement::Continue(std::string reason)
{
    _stateMachine.Continue(std::move(reason));
}

void LifecycleManagement::Stop(std::string reason)
{
    _stateMachine.Stop(std::move(reason));
}

void LifecycleManagement::StartAutonomous(std::string reason)
{
    _stateMachine.StartAutonomous(std::move(reason));
}

void LifecycleManagement::Error(std::string reason)
{
    _stateMachine.Error(std::move(reason));
}

void LifecycleManagement::AbortSimulation(std::string reason)
{
    _stateMachine.AbortSimulation(std::move(reason));
    if (_stateMachine.CurrentState() == LifecycleState::Error)
    {
        GetLogger()->Warn("AbortSimulation caused a transition to an error state");
    }
}

void LifecycleManagement::CommunicationInitializing(std::string reason)
{
    _stateMachine.CommunicationInitializing(std::move(reason));
}

// Callback handling
CallbackResult LifecycleManagement::HandleCommunicationReady()
{
    try
    {
        auto handlerDone = _lifecycleService->TriggerCommunicationReadyHandler();
        if (handlerDone)
        {
            return CallbackResult::Completed;
        }
        else
        {
            return CallbackResult::Deferred;
        }
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return CallbackResult::Error;
    }
}

bool LifecycleManagement::HandleStarting()
{
    try
    {
        _lifecycleService->TriggerStartingHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

bool LifecycleManagement::HandleStop()
{
    try
    {
        _lifecycleService->TriggerStopHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

bool LifecycleManagement::HandleShutdown()
{
    try
    {
        _lifecycleService->TriggerShutdownHandler();
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

void LifecycleManagement::AddAsyncSubscriptionsCompletionHandler(std::function<void()> handler)
{
    _lifecycleService->AddAsyncSubscriptionsCompletionHandler(std::move(handler));
}

auto LifecycleManagement::CurrentState() const -> LifecycleState
{
    return _stateMachine.CurrentState();
}

OperationMode LifecycleManagement::GetOperationMode() const
{
    return _lifecycleService->GetOperationMode();
}

auto LifecycleManagement::IsTimeSyncActive() const -> bool
{
    return _lifecycleService->IsTimeSyncActive();
}

void LifecycleManagement::ChangeParticipantState(ParticipantState newState, std::string reason)
{
    _lifecycleService->ChangeParticipantState(newState, std::move(reason));
}

auto LifecycleManagement::HandleAbort(ParticipantState lastState) -> bool
{
    try
    {
        _lifecycleService->TriggerAbortHandler(lastState);
        return true;
    }
    catch (const std::exception& e)
    {
        std::stringstream ss;
        ss << "Detected exception in callback:\n" << e.what();
        _logger->Warn(ss.str());
        return false;
    }
}

void LifecycleManagement::StartTime()
{
    (dynamic_cast<TimeSyncService*>(_lifecycleService->GetTimeSyncService()))->StartTime();
}

void LifecycleManagement::StopTime()
{
    (dynamic_cast<TimeSyncService*>(_lifecycleService->GetTimeSyncService()))->StopTime();
}

void LifecycleManagement::CallAfterAllParticipantsReplied(std::function<void()> handler)
{
    _participant->GetParticipantRepliesProcedure()->CallAfterAllParticipantsReplied(std::move(handler));
}

void LifecycleManagement::SetFinalStatePromise()
{
    _lifecycleService->SetFinalStatePromise();
}

auto LifecycleManagement::GetLogger() -> Services::Logging::ILogger*
{
    return _logger;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
