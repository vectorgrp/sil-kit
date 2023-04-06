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

#include <chrono>
#include <string>
#include <functional>

#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

enum class TimeProviderKind : uint8_t
{
    NoSync = 0,
    WallClock = 1,
    SyncTime = 2
};
/*!
* \brief Virtual time provider. Used for send timestamps.
* 
*/
class ITimeProvider
{
public:
    virtual ~ITimeProvider() {}
    //! \brief Get the current simulation time.
    virtual auto Now() const -> std::chrono::nanoseconds = 0;
    //! \brief Name of the time provider, for debugging purposes.
    virtual auto TimeProviderName() const -> const std::string& = 0;

    using NextSimStepHandler = std::function<void(std::chrono::nanoseconds now,
        std::chrono::nanoseconds duration)>;

    /*! \brief Register a handler that is executed when the next simulation step is started.
     *
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual auto AddNextSimStepHandler(NextSimStepHandler handler) -> HandlerId = 0;

    /*! \brief Remove NextSimStepHandler by HandlerId on this time provider.
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveNextSimStepHandler(HandlerId handlerId) = 0;

    virtual void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) = 0;
    virtual void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind) = 0;
    virtual void SetSynchronizeVirtualTime(bool isSynchronizingVirtualTime) = 0;
    virtual bool IsSynchronizingVirtualTime() const = 0;
};


} // namespace Orchestration
} // namespace Services
} // namespace SilKit
