// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include <functional>

namespace ib {
namespace mw {
namespace sync {

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
    //! \brief Register a handler that is executed when the current simulation step is finished.
    virtual void RegisterFinishedStepHandler(std::function<void(std::chrono::nanoseconds now)>) {};
    //! \brief Notify the TimeProvider about a finished simulation step.
    virtual void FinishedStep(std::chrono::nanoseconds now) {};
};


} // namespace sync
} // namespace mw
} // namespace ib
