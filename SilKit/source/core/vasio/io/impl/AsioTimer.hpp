#pragma once


#include "ITimer.hpp"

#include "asio.hpp"


namespace VSilKit {


class AsioTimer : public ITimer
{
    ITimerListener* _listener{nullptr};

    asio::steady_timer _timer;

public:
    explicit AsioTimer(asio::steady_timer timer);

    void SetListener(ITimerListener& listener) override;
    auto GetExpiry() const -> std::chrono::steady_clock::time_point override;
    void AsyncWaitFor(std::chrono::nanoseconds duration) override;
    void Shutdown() override;

private:
    void OnAsioAsyncWaitComplete(const asio::error_code& errorCode);
};


} // namespace VSilKit