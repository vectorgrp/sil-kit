// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "ITimer.hpp"

#include "asio.hpp"

#include <memory>


namespace VSilKit {


class AsioTimer final : public ITimer
{
    class Op : public std::enable_shared_from_this<Op>
    {
        std::atomic<AsioTimer*> _parent;
        asio::steady_timer _timer;

    public:
        explicit Op(AsioTimer& parent);

        void Initiate(std::chrono::nanoseconds duration);

        void Shutdown();
        void Abandon();

    private:
        void OnAsioAsyncWaitComplete(const asio::error_code& errorCode);
    };

    ITimerListener* _listener{nullptr};

    std::shared_ptr<asio::io_context> _asioIoContext;
    std::shared_ptr<Op> _op;

public:
    explicit AsioTimer(std::shared_ptr<asio::io_context> asioIoContext);
    ~AsioTimer() override;

    void SetListener(ITimerListener& listener) override;
    void AsyncWaitFor(std::chrono::nanoseconds duration) override;
    void Shutdown() override;
};


} // namespace VSilKit