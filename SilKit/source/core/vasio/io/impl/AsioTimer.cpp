#include "AsioTimer.hpp"

#include "util/Exceptions.hpp"

#include "silkit/SilKitMacros.hpp"


namespace VSilKit {


AsioTimer::AsioTimer(asio::steady_timer timer)
    : _timer{std::move(timer)}
{
}


void AsioTimer::SetListener(ITimerListener& listener)
{
    _listener = &listener;
}


auto AsioTimer::GetExpiry() const -> std::chrono::steady_clock::time_point
{
    return _timer.expiry();
}


void AsioTimer::AsyncWaitFor(std::chrono::nanoseconds duration)
{
    _timer.expires_after(duration);
    _timer.async_wait([this](const asio::error_code& e) {
        OnAsioAsyncWaitComplete(e);
    });
}


void AsioTimer::Shutdown()
{
    _timer.cancel();
}


void AsioTimer::OnAsioAsyncWaitComplete(const asio::error_code& errorCode)
{
    SILKIT_UNUSED_ARG(errorCode);

    asio::post(_timer.get_executor(), [&listener = *_listener, &self = *this] {
        listener.OnTimerExpired(self);
    });
}


} // namespace VSilKit
