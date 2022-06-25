// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <future>
#include <string>

#include "ib/mw/sync/ITimeSyncService.hpp"

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class ILifecycleService
{
public:
    using CommunicationReadyHandlerT = std::function<void()>;
    using StartingHandlerT = std::function<void()>;
    using StopHandlerT = std::function<void()>;
    using ShutdownHandlerT = std::function<void()>;

public:
    /*! \brief Register a callback that is executed once communication with 
     * controllers is possible.
     *
     * The handler is called after \ref SystemState::CommunicationReady
     * is reached.
     * TODO fill in on which thread this is executed.
     * After the handler has been processed, the participant
     * switches to the \ref ParticipantState::ReadyToRun state.
     */
    virtual void SetCommunicationReadyHandler(CommunicationReadyHandlerT handler) = 0;

    /*! \brief (Asynchronous participants only) Register a callback that is executed once directly before the participant enters ParticipantState::Run.
     *
     * This handler is triggered just before the participant changes to ParticipantState::Running.
     * It is only triggered if the participant does NOT use virtual time synchronization.
     * It does not block other participants from changing to ParticipantState::Running and should only be used for lightweight operations such as starting timers.
     * TODO fill in on which thread this is executed.
     * After the handler has been processed, the participant
     * switches to the \ref ParticipantState::Running state.
     */
    virtual void SetStartingHandler(StartingHandlerT handler) = 0;

    /*! \brief Register a callback that is executed on simulation stop.
     *
     * The handler is called when a \ref SystemCommand::Kind::Stop has been
     * received. It is executed in the context of the middleware
     * thread that received the command. After the handler has been
     * processed, the participant switches to the
     * \ref ParticipantState::Stopped state.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetStopHandler(StopHandlerT handler) = 0;

    /*! \brief Register a callback that is executed on simulation shutdown.
     *
     * The handler is called when the \ref SystemCommand::Kind::Shutdown
     * has been received. It is executed in the context of the middleware
     * thread that received the command. After the handler has been
     * processed, the participant switches to the
     * \ref ParticipantState::Shutdown state and is allowed to terminate.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetShutdownHandler(ShutdownHandlerT handler) = 0;

    /*! \brief Start non blocking operation without virtual time synchronization, returns immediately.
     *
     * Executes simulation without virtual time synchronization until shutdown is received. The simulation
     * task is executed in the context of the middleware thread that
     * receives the grant or tick.
     *
     * \return Future that will hold the final state of the participant
     * once the LifecycleService finishes operation.
     */
    virtual auto ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop,
                                            bool isRequiredParticipant) -> std::future<ParticipantState> = 0;
    virtual auto ExecuteLifecycleNoSyncTime(bool hasCoordinatedSimulationStart, bool hasCoordinatedSimulationStop)
        -> std::future<ParticipantState> = 0;

    
    /*! \brief Start non blocking operation with virtual time synchronization, returns immediately.
     *
     * Executes simulation with virtual time synchronization until shutdown is received. The simulation
     * task is executed in the context of the middleware thread that
     * receives the grant or tick.
     *
     * \return Future that will hold the final state of the participant
     * once the LifecycleService finishes operation.
     */
    virtual auto ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService, bool hasCoordinatedSimulationStart,
                                              bool hasCoordinatedSimulationStop, bool isRequiredParticipant)
        -> std::future<ParticipantState> = 0;
    virtual auto ExecuteLifecycleWithSyncTime(ITimeSyncService* timeSyncService, bool hasCoordinatedSimulationStart,
                                              bool hasCoordinatedSimulationStop) -> std::future<ParticipantState> = 0;


    /*! \brief Abort current simulation run due to an error.
     *
     * Switch to the \ref ParticipantState::Error state and
     * report the error message in the IB system.
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

    // TODO FIXME Move elsewhere!
    ////! \brief Experimental API - Not final yet.
    //virtual void LogCurrentPerformanceStats() = 0;
    //
    virtual auto GetTimeSyncService() const -> ITimeSyncService* = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
