// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <future>
#include <string>

#include "SyncDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class ITimeSyncService
{
public:
    using SimulationStepT = std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)>;
public:
    virtual ~ITimeSyncService() = default;
public:
    /*! \brief Set the task to be executed with each grant / tick
     *
     * Can be changed at runtime. Execution context depends on the run type.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetSimulationStepHandler(SimulationStepT task, std::chrono::nanoseconds initialStepSize) = 0;
    /*! \brief Set the task to be executed with each grant / tick
     *
     * Can be changed at runtime. Execution context depends on the run type.
     * Execution will perform one simulation step at a time.
     * CompleteSimulationStep is required to signal completion of the simulation step.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetSimulationStepHandlerAsync(SimulationStepT task, std::chrono::nanoseconds initialStepSize) = 0;
    /*! \brief Signal that the current simulation task is finished and the next simulation step can be processed.
     *
     * This method should only be used after calling SetSimulationStepHandlerAsync.
     * Otherwise, undefined runtime behavior will result.
     */
    virtual void CompleteSimulationStep() = 0;

    //[[deprecated]]
    virtual void SetSimulationStepHandler(std::function<void(std::chrono::nanoseconds now)> task,
                                          std::chrono::nanoseconds initialStepSize) = 0;


    /*! \brief Get the current simulation time
     */
    virtual auto Now() const -> std::chrono::nanoseconds = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
