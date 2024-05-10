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

#include <functional>
#include <future>
#include <string>

#include "OrchestrationDatatypes.hpp"

#include "silkit/services/orchestration/ITimeSyncService.hpp"
#include "silkit/participant/exception.hpp"

#include "silkit/capi/Participant.h"

namespace SilKit {
namespace Services {
namespace Orchestration {

/*! This handler is called after \ref ParticipantState::CommunicationInitialized is reached
 *  Cf., \ref ILifecycleService::SetCommunicationReadyHandler(CommunicationReadyHandler);
 */
using CommunicationReadyHandler = std::function<void()>;

/*! This handler is called before the participant enters \ref ParticipantState::Running.
 *  Cf., \ref ILifecycleService::SetStartingHandler(StartingHandler);
 */
using StartingHandler = std::function<void()>;

/*! This handler is called after \ref ParticipantState::Stopping is reached
 *  Cf., \ref ILifecycleService::SetStopHandler(StopHandler);
 */
using StopHandler = std::function<void()>;

/*! This handler is called after \ref ParticipantState::ShuttingDown is reached
 *  Cf., \ref ILifecycleService::SetShutdownHandler(ShutdownHandler);
 */
using ShutdownHandler = std::function<void()>;

/*! This handler is called after \ref ParticipantState::Aborting is reached
 *  Cf., \ref ILifecycleService::SetAbortHandler(AbortHandler);
 */
using AbortHandler = std::function<void(ParticipantState /*lastState*/)>;


class ILifecycleService
{
public:
    virtual ~ILifecycleService() = default;

public:

    /*! \brief Register a callback that is executed once all communication channels between participants
     *          with a lifecycle have been set up and are ready for communication.
     *
     * The handler is called when the participant reaches the \ref ParticipantState::CommunicationInitialized.
     * This handler is invoked in an internal thread, and receiving messages during the handler invocation is not possible.
     * For an asynchronous invocation, see \ref SetCommunicationReadyHandlerAsync and \ref CompleteCommunicationReadyHandlerAsync.
     * After the handler has been processed, the participant switches to the \ref ParticipantState::ReadyToRun state.
     */
    virtual void SetCommunicationReadyHandler(CommunicationReadyHandler handler) = 0;

    /*! \brief Register a callback that is executed once all communication channels between participants
    *          with a lifecycle have been set up and are ready for communication.
    *
    * The handler is called when the participant reaches the \ref ParticipantState::CommunicationInitialized.
    * Even after the callback returns, the participant will stay in the ParticipantState::CommunicationInitialized until the 
    * CompleteCommunicationReadyHandlerAsync() method is called. Only then will the participant switch to the ParticipantState::ReadyToRun.
    * Note that CompleteCommunicationReadyHandlerAsync may not be called from within any CommunicationReadyHandler.
    * The CommunicationReadyHandler is executed in an internal thread and must not be blocked by the user.
    */ 
    virtual void SetCommunicationReadyHandlerAsync(CommunicationReadyHandler handler) = 0;

    /*! \brief Notify that the async CommunicationReadyHandler is completed.
     *
     * This method must not be invoked from within a CommunicationReadyHandler.
     */
    virtual void CompleteCommunicationReadyHandlerAsync() = 0;

    /*! \brief (Participants without TimeSyncService only) Register a callback that is executed once directly before the participant enters ParticipantState::Running.
     *
     * This handler is triggered just before the participant changes to ParticipantState::Running.
     * It is only triggered if the participant does NOT use virtual time synchronization.
     * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
     * It is executed in the context of an internal thread that received the command. 
     * After the handler has been processed, the participant
     * switches to the \ref ParticipantState::Running state.
     */
    virtual void SetStartingHandler(StartingHandler handler) = 0;

    /*! \brief Register a callback that is executed on simulation stop.
     *
     * The handler is called when the participant enters the \ref ParticipantState::Stopping state.
     * It is executed in the context of an internal
     * thread that received the command. After the handler has been
     * processed, the participant switches to the
     * \ref ParticipantState::Stopped state.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetStopHandler(StopHandler handler) = 0;

    /*! \brief Register a callback that is executed on simulation shutdown.
     *
     * The handler is called when the participant enters the \ref ParticipantState::ShuttingDown state.
     * It is executed in the context of an internal
     * thread that received the command. After the handler has been
     * processed, the participant switches to the
     * \ref ParticipantState::Shutdown state and is allowed to terminate.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetShutdownHandler(ShutdownHandler handler) = 0;

    /*! \brief Register a callback that is executed on simulation abort.
     *
     * The handler is called when the participant enters the \ref ParticipantState::Aborting.
     * It is executed in the context of an internal
     * thread that received the command. After the handler has been
     * processed, the participant switches to the
     * \ref ParticipantState::Shutdown state and is allowed to terminate.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetAbortHandler(AbortHandler handler) = 0;

    /*! \brief Start non blocking operation; returns immediately.
     *
     * Starts simulation until the simulation is stopped or aborted.
     *
     * \return Future that will hold the final state of the participant
     * once the LifecycleService finishes operation.
     */
    virtual auto StartLifecycle() -> std::future<ParticipantState> = 0;

    /*! \brief Abort current simulation run due to an error.
     *
     * Switch to the \ref ParticipantState::Error state and
     * report the error message in the SIL Kit system.
     */
    virtual void ReportError(std::string errorMsg) = 0;

    /*! \brief Pause execution of the participant
     *
     * Switch to \ref ParticipantState::Paused due to the provided \p reason.
     *
     * When a client is in state \ref ParticipantState::Paused,
     * it must not be considered as unresponsive even if a
     * health monitoring related timeout occurs.
     *
     * Precondition: State() == \ref ParticipantState::Running
     */
    virtual void Pause(std::string reason) = 0;

    /*! \brief Switch back to \ref ParticipantState::Running
     * after having paused.
     *
     * Precondition: State() == \ref ParticipantState::Paused
     */
    virtual void Continue() = 0;

    /*! \brief Stop execution of the participant.
     *
     * Allows the participant to exit the RunAsync loop, e.g., if it
     * is unable to further progress its simulation.
     *
     * Calls the StopHandler and then switches to the
     * \ref ParticipantState::Stopped state.
     *
     * NB: In general, Stop should not be called by the participants
     * as the end of simulation is governed by the central execution
     * controller. This method should only be used if the client
     * cannot participate in the system simulation anymore.
     *
     * Precondition: State() == \ref ParticipantState::Running
     */
    virtual void Stop(std::string reason) = 0;

    /*! \brief Get the current participant status
    */
    virtual auto State() const -> ParticipantState = 0;

    /*! \brief Get the current participant status
    */
    virtual auto Status() const -> const ParticipantStatus& = 0;

    /*! \brief Return the  ITimeSyncService for the current ILifecycleService.
    */
    virtual auto CreateTimeSyncService() -> ITimeSyncService* = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
