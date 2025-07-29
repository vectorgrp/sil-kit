// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>
#include <memory>
#include <mutex>

#include "ITimeProvider.hpp"
#include "Timer.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

struct ITimeProviderImplListener;

struct ITimeProviderImpl
{
    virtual ~ITimeProviderImpl() = default;

    virtual auto TimeProviderName() const -> const std::string& = 0;

    virtual void SetActive(bool value) = 0;

    virtual void OnHandlerAdded() = 0;

    virtual auto Now() const -> std::chrono::nanoseconds = 0;

    virtual void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) = 0;
};

struct ITimeProviderImplListener
{
    virtual ~ITimeProviderImplListener() = default;

    virtual void OnTick(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) = 0;
};

class TimeProvider
    : public ITimeProvider
    , private ITimeProviderImplListener
{
public:
    TimeProvider();
    ~TimeProvider() override = default;

public:
    //ITimeProvider
    inline auto Now() const -> std::chrono::nanoseconds override;
    inline auto TimeProviderName() const -> const std::string& override;
    inline HandlerId AddNextSimStepHandler(NextSimStepHandler handler) override;
    inline void RemoveNextSimStepHandler(HandlerId handlerId) override;
    inline void SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) override;
    inline void SetSynchronizeVirtualTime(bool isSynchronizingVirtualTime) override;
    inline bool IsSynchronizingVirtualTime() const override;

    void ConfigureTimeProvider(Orchestration::TimeProviderKind timeProviderKind) override;

private:
    void OnTick(std::chrono::nanoseconds now, std::chrono::nanoseconds duration) final;

private: //Members
    mutable std::recursive_mutex _mutex;
    Util::Handlers<NextSimStepHandler> _handlers;
    bool _isSynchronizingVirtualTime{false};
    std::unique_ptr<ITimeProviderImpl> _currentProvider;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
/////////////////////////////////////////////////////////////////////


auto TimeProvider::Now() const -> std::chrono::nanoseconds
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->Now();
}

auto TimeProvider::TimeProviderName() const -> const std::string&
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _currentProvider->TimeProviderName();
}

HandlerId TimeProvider::AddNextSimStepHandler(NextSimStepHandler handler)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    const auto handlerId{_handlers.Add(std::move(handler))};
    _currentProvider->OnHandlerAdded();
    return handlerId;
}

void TimeProvider::RemoveNextSimStepHandler(HandlerId handlerId)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _handlers.Remove(handlerId);
}

void TimeProvider::SetTime(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _currentProvider->SetTime(now, duration);
}

void TimeProvider::SetSynchronizeVirtualTime(bool isSynchronizingVirtualTime)
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _isSynchronizingVirtualTime = isSynchronizingVirtualTime;
}

bool TimeProvider::IsSynchronizingVirtualTime() const
{
    std::unique_lock<decltype(_mutex)> lock{_mutex};
    return _isSynchronizingVirtualTime;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
