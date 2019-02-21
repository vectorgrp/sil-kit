// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <future>
#include <string>

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class IParticipantController
{
public:
    using InitHandlerT = std::function<void(ParticipantCommand)>;
    using StopHandlerT = std::function<void()>;
    using ShutdownHandlerT = std::function<void()>;
    using SimTaskT = std::function<void(std::chrono::nanoseconds now)>;
    
public:
    /*! \brief Register a callback to perform initialization.
     *
     * The handler is called when an /Initialize/ or /ReInitialize/
     * command has been received. The callback is executed in the
     * context of the middleware thread that received the
     * command. After the handler has been processed, the participant
     * switches to the /Initialized/ state.
     */
    virtual void SetInitHandler(InitHandlerT handler) = 0;

    /*! \brief Register a callback that is executed on simulation stop.
     *
     * The handler is called when a /Stop/ ommand has been
     * received. It is executed in the context of the middleware
     * thread that received the command. After the handler has been
     * processed, the participant switches to the /Stopped/ state.
     * 
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetStopHandler(StopHandlerT handler) = 0;

    /*! \brief Register a callback that is executed on simulation shutdown.
     *
     * The handler is called when the /Shutdown/ ommand has been
     * received. It is executed in the context of the middleware
     * thread that received the command. After the handler has been
     * processed, the participant switches to the /Shutdown/ state and
     * is allowed to terminate.
     * 
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetShutdownHandler(ShutdownHandlerT handler) = 0;
    
    /*! \brief Set the task to be executed with each grant / tick
     *
     * Can be changed at runtime. Execution context depends on the run type.
     * 
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetSimulationTask(SimTaskT task) = 0;

    /*! \brief Enable coldswap for this participant
     *
     * With coldswap enabled, Run()/RunAsyc() will finish in state
     * ParticipantState::ColdswapShutdown to indicate that it is
     * now safe to disconnect and reconnect.
     *
     * By default, coldswap is disabled, and the participant will
     * not disconnect during a coldswap.
     */
    virtual void EnableColdswap() = 0;

    /*! \brief Set the simulation duration to be requested
     *
     * Can only be used with time quantum synchronization.
     */
    virtual void SetPeriod(std::chrono::nanoseconds period) = 0;

    /*! \brief Set the simulation time for the next pending event.
     *
     * Can only be used with event triggered synchronization.
     */
    virtual void SetEarliestEventTime(std::chrono::nanoseconds eventTime) = 0;

    /*! \brief Start blocking operation
     *
     * Executes simulation until shutdown is received. The simulation
     * task is executed in the context of the calling thread.
     *
     * \return Final state of the participant. In normal operation,
     * should be ParticipantState::Shutdown.
     */
    virtual auto Run() -> ParticipantState = 0;

    /*! \brief Start non blocking operation, returns immediately.
     *
     * Executes simulation until shutdown is received. The simulation
     * task is executed in the context of the middleware thread that
     * receives the grant or tick.
     *
     * NB: RunAsync() cannot be used with SyncPolicy::Strict, which
     * will inherently lead to a deadlock!
     * 
     * \return Future that will hold the final state of the participant
     * once the ParticipantController finishes operation.
     */
    virtual auto RunAsync() -> std::future<ParticipantState> = 0;

    /*! \brief Abort current simulation run due to an error.
     *
     * Switch to the /Error/ state and report the error message in the
     * IB system.
     */
    virtual void ReportError(std::string errorMsg) = 0;

    /*! \brief Pause execution of the participant
     *
     * Switch to the Paused state due to the provided \param
     * reason.
     *
     * When a client is in state Paused, it must not be considered as
     * unresponsive even if a health monitoring related timeout
     * occurs.
     *
     * Precondition: State() == ParticipantState::Running
     */
    virtual void Pause(std::string reason) = 0;

    /*! \brief Switch back to Running after having paused.
     *
     * Precondition: State() == ParticipantState::Paused
     */
    virtual void Continue() = 0;
    
    /*! \brief Stop execution of the participant.
     *
     * Allows the participant to exit the RunAsync loop, e.g., if it
     * is unable to further progress its simulation.
     * 
     * Calls the StopHandler and then switches to the /Stopped/ state.
     *
     * NB: In general, Stop should not be called by the participants
     * as the end of simulation is governed by the central execution
     * controller. This method should only be used if the client
     * cannot participante in the system simulation anymore.
     *
     * Precondition: State() == ParticipantState::Running
     */
    virtual void Stop(std::string reason) = 0;

    /*! \brief Get the current participant state
     */
    virtual auto State() const -> ParticipantState = 0;

    /*! \brief Get the current participant status
    */
    virtual auto Status() const -> const ParticipantStatus& = 0;

    /*! \brief Refresh the current status without modifying the state
     *
     * This method only refreshes the field ParticipantStatus::refreshTime
     * to the current wall clock time. All other fields are left unchanged.
     * This method and field can be used to indicate that the participant is
     * still alive and operational.
     */
    virtual void RefreshStatus() = 0;

    /*! \brief Get the current simulation time
     */
    virtual auto Now() const -> std::chrono::nanoseconds = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
