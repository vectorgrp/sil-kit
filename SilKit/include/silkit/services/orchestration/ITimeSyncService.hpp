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

#include "OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class ITimeSyncService
{
public:
    using SimulationStepHandler = std::function<void(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)>;

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
    virtual void SetSimulationStepHandler(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) = 0;
    /*! \brief Set the task to be executed with each grant / tick
     *
     * Can be changed at runtime. Execution context depends on the run type.
     * Execution will perform one simulation step at a time.
     * CompleteSimulationStep is required to signal completion of the simulation step.
     *
     * Throwing an error inside the handler will cause a call to
     * ReportError().
     */
    virtual void SetSimulationStepHandlerAsync(SimulationStepHandler task, std::chrono::nanoseconds initialStepSize) = 0;
    /*! \brief Signal that the current simulation task is finished and the next simulation step can be processed.
     *
     * This method should only be used after calling SetSimulationStepHandlerAsync.
     * Otherwise, undefined runtime behavior will result.
     */
    virtual void CompleteSimulationStep() = 0;

    /*! \brief Get the current simulation time
     */
    virtual auto Now() const -> std::chrono::nanoseconds = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
