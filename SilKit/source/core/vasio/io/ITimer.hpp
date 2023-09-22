#pragma once


#include <chrono>


namespace VSilKit {


struct ITimerListener;


struct ITimer
{
    virtual ~ITimer() = default;

    virtual void SetListener(ITimerListener& listener) = 0;

    virtual auto GetExpiry() const -> std::chrono::steady_clock::time_point = 0;

    virtual void AsyncWaitFor(std::chrono::nanoseconds duration) = 0;

    virtual void Shutdown() = 0;
};


struct ITimerListener
{
    virtual ~ITimerListener() = default;

    virtual void OnTimerExpired(ITimer& timer) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ITimer;
using VSilKit::ITimerListener;
} // namespace Core
} // namespace SilKit
