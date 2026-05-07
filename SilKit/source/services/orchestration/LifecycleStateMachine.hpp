// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <initializer_list>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

#include <fmt/format.h>

#include "LifecycleState.hpp"
#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

template <class Effects>
class LifecycleStateMachine
{
public:
    explicit LifecycleStateMachine(Effects& effects)
        : _effects{effects}
    {
    }

    auto CurrentState() const -> LifecycleState
    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};
        return _state;
    }

    void Initialize(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleInitialize, std::move(reason));
    }

    void ServicesCreated(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleServicesCreated, std::move(reason));
    }

    void CommunicationInitializing(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleCommunicationInitializing, std::move(reason));
    }

    void CommunicationInitialized(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleCommunicationInitialized, std::move(reason));
    }

    void CompleteCommunicationReadyHandler(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleCompleteCommunicationReadyHandler, std::move(reason));
    }

    void ReadyToRun(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleReadyToRun, std::move(reason));
    }

    void RunSimulation(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleRunSimulation, std::move(reason));
    }

    void Pause(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandlePause, std::move(reason));
    }

    void Continue(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleContinue, std::move(reason));
    }

    void Stop(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleStop, std::move(reason));
    }

    void Shutdown(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleShutdown, std::move(reason));
    }

    void AbortSimulation(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleAbortSimulation, std::move(reason));
    }

    void ResolveAbortSimulation(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleResolveAbortSimulation, std::move(reason));
    }

    void Error(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleError, std::move(reason));
    }

    void Restart(std::string reason)
    {
        Dispatch(&LifecycleStateMachine::HandleRestart, std::move(reason));
    }

    void StartAutonomous(std::string reason)
    {
        ServicesCreated(std::move(reason));
    }

private:
    using Handler = void (LifecycleStateMachine::*)(std::string);

private:
    void Dispatch(Handler handler, std::string reason)
    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};
        (this->*handler)(std::move(reason));
    }

    void TransitionTo(LifecycleState newState, std::string reason)
    {
        UpdateState(newState);
        PublishCurrentState(std::move(reason));
    }

    void TransitionToAndForward(LifecycleState newState, Handler handler, std::string reason)
    {
        UpdateState(newState);
        PublishCurrentState(reason);
        (this->*handler)(std::move(reason));
    }

    void UpdateState(LifecycleState newState)
    {
        if (newState == LifecycleState::Aborting)
        {
            _lastBeforeAbortingState = _state;
        }

        _state = newState;
    }

    void PublishCurrentState(std::string reason)
    {
        _effects.ChangeParticipantState(ToParticipantState(_state), std::move(reason));
    }

    void InvalidStateTransition(const char* transitionName, bool triggerErrorState, std::string originalReason)
    {
        std::stringstream ss;
        ss << "Detected invalid state transition.\n"
           << "Current state: " << to_string(_state) << "\n"
           << "Requested transition: " << transitionName << "\n"
           << "Original reason: " << originalReason;

        if (triggerErrorState)
        {
            HandleError(ss.str());
        }
        else
        {
            _effects.GetLogger()->Warn(ss.str());
        }
    }

    void ProcessAbortCommandInCallback()
    {
        auto reason = fmt::format("Received SystemCommand::AbortSimulation during {} callback", to_string(_state));
        ProcessAbortCommand(std::move(reason));
    }

    void ProcessAbortCommand(std::string reason)
    {
        _commReadyAbortRequested = false;
        _startingAbortRequested = false;
        _stoppingAbortRequested = false;
        TransitionToAndForward(LifecycleState::Aborting, &LifecycleStateMachine::HandleResolveAbortSimulation,
                               std::move(reason));
    }

    void HandleInitialize(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Invalid:
            TransitionTo(LifecycleState::ServicesCreated, std::move(reason));
            return;
        case LifecycleState::Shutdown:
        case LifecycleState::Aborting:
        case LifecycleState::Error:
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleServicesCreated(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::ServicesCreated:
            _effects.CallAfterAllParticipantsReplied([this, reason]() mutable {
                std::unique_lock<decltype(_mutex)> lock{_mutex};
                TransitionToAndForward(LifecycleState::CommunicationInitializing,
                                       &LifecycleStateMachine::HandleServicesCreated, std::move(reason));
            });
            return;
        case LifecycleState::CommunicationInitializing:
            HandleCommunicationInitializing(std::move(reason));
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleCommunicationInitializing(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::CommunicationInitializing:
            _effects.AddAsyncSubscriptionsCompletionHandler([this, reason]() mutable {
                std::unique_lock<decltype(_mutex)> lock{_mutex};
                TransitionToAndForward(LifecycleState::CommunicationInitialized,
                                       &LifecycleStateMachine::HandleCommunicationInitializing, std::move(reason));
            });
            return;
        case LifecycleState::CommunicationInitialized:
            if (_effects.GetOperationMode() == OperationMode::Autonomous)
            {
                HandleCommunicationInitialized("CommunicationInitialized for autonomous participant.");
            }
            return;
        case LifecycleState::ReadyToRun:
        case LifecycleState::Running:
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleCommunicationInitialized(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::CommunicationInitialized:
            _commReadyHandlerExecuting = true;
            switch (_effects.HandleCommunicationReady())
            {
            case CallbackResult::Error:
                if (_commReadyAbortRequested)
                {
                    ProcessAbortCommandInCallback();
                }
                else
                {
                    TransitionTo(LifecycleState::Error, "Exception during CommunicationReadyHandle execution.");
                }
                break;
            case CallbackResult::Completed:
                if (_commReadyAbortRequested)
                {
                    ProcessAbortCommandInCallback();
                }
                else
                {
                    HandleCompleteCommunicationReadyHandler(std::move(reason));
                }
                break;
            case CallbackResult::Deferred:
                _effects.GetLogger()->Debug("Deferred CommunicationReady callback.");
                break;
            }
            _commReadyHandlerExecuting = false;
            return;
        case LifecycleState::ReadyToRun:
            if (_effects.GetOperationMode() == OperationMode::Autonomous)
            {
                HandleReadyToRun("ReadyToRun for autonomous participant.");
            }
            return;
        case LifecycleState::Shutdown:
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleCompleteCommunicationReadyHandler(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::CommunicationInitialized:
            TransitionToAndForward(LifecycleState::ReadyToRun, &LifecycleStateMachine::HandleCommunicationInitialized,
                                   std::move(reason));
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleReadyToRun(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::ReadyToRun:
            if (!_effects.IsTimeSyncActive())
            {
                _startingHandlerExecuting = true;
                const auto success = _effects.HandleStarting();
                if (success)
                {
                    if (_startingAbortRequested)
                    {
                        ProcessAbortCommandInCallback();
                    }
                    else
                    {
                        TransitionTo(LifecycleState::Running, "Finished StartingHandler execution.");
                    }
                }
                else
                {
                    if (_startingAbortRequested)
                    {
                        ProcessAbortCommandInCallback();
                    }
                    else
                    {
                        TransitionTo(LifecycleState::Error, "Exception during StartingHandler execution.");
                    }
                }
                _startingHandlerExecuting = false;
            }
            else
            {
                if (_startingAbortRequested)
                {
                    ProcessAbortCommandInCallback();
                }
                else
                {
                    TransitionToAndForward(LifecycleState::Running, &LifecycleStateMachine::HandleReadyToRun,
                                           std::move(reason));
                }
            }
            return;
        case LifecycleState::Running:
            _effects.StartTime();
            return;
        case LifecycleState::Shutdown:
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleRunSimulation(std::string reason)
    {
        InvalidStateTransition(__FUNCTION__, true, std::move(reason));
    }

    void HandlePause(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Running:
            TransitionTo(LifecycleState::Paused, std::move(reason));
            return;
        case LifecycleState::Paused:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleContinue(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Running:
            InvalidStateTransition(__FUNCTION__, false, std::move(reason));
            return;
        case LifecycleState::Paused:
            TransitionTo(LifecycleState::Running, std::move(reason));
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleStop(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Running:
            _effects.StopTime();
            TransitionToAndForward(LifecycleState::Stopping, &LifecycleStateMachine::HandleStop, std::move(reason));
            return;
        case LifecycleState::Paused:
            TransitionToAndForward(LifecycleState::Stopping, &LifecycleStateMachine::HandleStop, std::move(reason));
            return;
        case LifecycleState::Stopping:
            if (_effects.HandleStop())
            {
                if (_stoppingAbortRequested)
                {
                    ProcessAbortCommandInCallback();
                }
                else
                {
                    TransitionToAndForward(LifecycleState::Stopped, &LifecycleStateMachine::HandleStop, std::move(reason));
                }
            }
            else
            {
                if (_stoppingAbortRequested)
                {
                    ProcessAbortCommandInCallback();
                }
                else
                {
                    TransitionTo(LifecycleState::Error, "Exception during StopHandler execution.");
                }
            }
            return;
        case LifecycleState::Stopped:
            _effects.CallAfterAllParticipantsReplied([this, reason]() mutable {
                std::unique_lock<decltype(_mutex)> lock{_mutex};
                HandleShutdown(std::move(reason));
            });
            return;
        case LifecycleState::ShuttingDown:
        case LifecycleState::Shutdown:
            return;
        case LifecycleState::Error:
            HandleShutdown(std::move(reason));
            return;
        default:
            InvalidStateTransition(__FUNCTION__, false, std::move(reason));
            return;
        }
    }

    void HandleShutdown(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Stopped:
        case LifecycleState::Error:
            TransitionToAndForward(LifecycleState::ShuttingDown, &LifecycleStateMachine::HandleShutdown,
                                   std::move(reason));
            return;
        case LifecycleState::ShuttingDown:
            if (!_effects.HandleShutdown())
            {
                _effects.GetLogger()->Warn(
                    "ShutdownHandler threw an exception. This is ignored. The participant will now shut down.");
            }
            TransitionToAndForward(LifecycleState::Shutdown, &LifecycleStateMachine::HandleShutdown, std::move(reason));
            return;
        case LifecycleState::Aborting:
            TransitionToAndForward(LifecycleState::Shutdown, &LifecycleStateMachine::HandleShutdown, std::move(reason));
            return;
        case LifecycleState::Shutdown:
            _effects.CallAfterAllParticipantsReplied([this, reason]() mutable {
                std::unique_lock<decltype(_mutex)> lock{_mutex};
                if (_state != LifecycleState::Error)
                {
                    _effects.NotifyShutdownInConnection();
                    _effects.SetFinalStatePromise();
                }
                else
                {
                    _effects.GetLogger()->Warn(
                        fmt::format("lifecycle failed to shut down correctly - original shutdown reason was '{}'.",
                                    reason));
                }
            });
            return;
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

    void HandleAbortSimulation(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Invalid:
        case LifecycleState::ServicesCreated:
        case LifecycleState::CommunicationInitializing:
        case LifecycleState::Stopped:
        case LifecycleState::Error:
            HandleResolveAbortSimulation(std::move(reason));
            return;
        case LifecycleState::CommunicationInitialized:
            if (_commReadyHandlerExecuting)
            {
                _commReadyAbortRequested = true;
            }
            else
            {
                HandleResolveAbortSimulation("Received SystemCommand::AbortSimulation.");
            }
            return;
        case LifecycleState::ReadyToRun:
            if (_startingHandlerExecuting)
            {
                _startingAbortRequested = true;
            }
            else
            {
                HandleResolveAbortSimulation(std::move(reason));
            }
            return;
        case LifecycleState::Running:
            _effects.StopTime();
            HandleResolveAbortSimulation(std::move(reason));
            return;
        case LifecycleState::Paused:
            HandleResolveAbortSimulation("Received abort simulation.");
            return;
        case LifecycleState::Stopping:
            _stoppingAbortRequested = true;
            return;
        case LifecycleState::ShuttingDown:
            HandleResolveAbortSimulation(std::string{});
            return;
        case LifecycleState::Shutdown:
            HandleResolveAbortSimulation(std::string{});
            return;
        case LifecycleState::Aborting:
            return;
        }
    }

    void HandleResolveAbortSimulation(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Invalid:
        case LifecycleState::ServicesCreated:
        case LifecycleState::CommunicationInitializing:
        case LifecycleState::CommunicationInitialized:
        case LifecycleState::ReadyToRun:
        case LifecycleState::Running:
        case LifecycleState::Paused:
        case LifecycleState::Stopping:
        case LifecycleState::Stopped:
        case LifecycleState::Error:
            ProcessAbortCommand(std::move(reason));
            return;
        case LifecycleState::ShuttingDown:
            _effects.GetLogger()->Info("Received abort signal while shutting down - ignoring abort.");
            return;
        case LifecycleState::Shutdown:
            _effects.GetLogger()->Info("Received abort signal after shutdown - ignoring abort.");
            return;
        case LifecycleState::Aborting:
            if (_effects.HandleAbort(ToParticipantState(_lastBeforeAbortingState)))
            {
                // NOP
            }
            else
            {
                _effects.GetLogger()->Warn(
                    "ShutdownHandler threw an exception. This is ignored. The participant will now shut down.");
            }
            HandleShutdown(std::move(reason));
            return;
        }
    }

    void HandleError(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Shutdown:
            return;
        case LifecycleState::Error:
            _effects.GetLogger()->Warn("Received error transition within error state. Original reason: "
                                       + std::move(reason));
            return;
        default:
            TransitionTo(LifecycleState::Error, std::move(reason));
            return;
        }
    }

    void HandleRestart(std::string reason)
    {
        switch (_state)
        {
        case LifecycleState::Stopped:
        case LifecycleState::Error:
            throw SilKitError("Restart is currently not supported.");
        default:
            InvalidStateTransition(__FUNCTION__, true, std::move(reason));
            return;
        }
    }

private:
    Effects& _effects;
    mutable std::recursive_mutex _mutex;
    LifecycleState _state{LifecycleState::Invalid};
    LifecycleState _lastBeforeAbortingState{LifecycleState::Invalid};
    bool _commReadyHandlerExecuting{false};
    bool _commReadyAbortRequested{false};
    bool _startingHandlerExecuting{false};
    bool _startingAbortRequested{false};
    bool _stoppingAbortRequested{false};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
