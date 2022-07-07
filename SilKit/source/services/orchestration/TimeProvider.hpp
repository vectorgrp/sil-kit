// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include <memory>

#include "ITimeProvider.hpp"
#include "Timer.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {


class TimeProvider : public ITimeProvider
{
public:
    //CTor
    virtual ~TimeProvider() = default;
    TimeProvider();
public:
    //ITimeProvider
    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandlerT handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;
    inline void SetSynchronized(bool isSynchronized) override;
    inline bool IsSynchronized() const override;

    void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind) override;

private:
    std::unique_ptr<ITimeProvider> _currentProvider;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////


auto TimeProvider::Now() const -> std::chrono::nanoseconds
{
    return _currentProvider->Now();
}
auto TimeProvider::TimeProviderName() const -> const std::string&
{
    return _currentProvider->TimeProviderName();
}
HandlerId TimeProvider::AddNextSimStepHandler(NextSimStepHandlerT handler)
{
    return _currentProvider->AddNextSimStepHandler(std::move(handler));
}
void TimeProvider::RemoveNextSimStepHandler(HandlerId handlerId)
{
    _currentProvider->RemoveNextSimStepHandler(handlerId);
}
void TimeProvider::SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    _currentProvider->SetTime(now, duration);
}
void TimeProvider::SetSynchronized(bool isSynchronized)
{
    _currentProvider->SetSynchronized(isSynchronized);
}
bool TimeProvider::IsSynchronized() const
{
    return _currentProvider->IsSynchronized();
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
