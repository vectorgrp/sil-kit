// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "AsioTimer.hpp"

#include "util/Exceptions.hpp"

#include "silkit/SilKitMacros.hpp"


namespace VSilKit {


AsioTimer::AsioTimer(std::shared_ptr<asio::io_context> asioIoContext)
    : _asioIoContext{std::move(asioIoContext)}
    , _op{std::make_shared<Op>(*this)}
{
}


AsioTimer::~AsioTimer()
{
    _op->Abandon();
    _op->Shutdown();
}


void AsioTimer::SetListener(ITimerListener& listener)
{
    _listener = &listener;
}


void AsioTimer::AsyncWaitFor(std::chrono::nanoseconds duration)
{
    _op->Initiate(duration);
}


void AsioTimer::Shutdown()
{
    _op->Shutdown();
}


AsioTimer::Op::Op(VSilKit::AsioTimer& parent)
    : _parent{&parent}
    , _timer{*parent._asioIoContext}
{
}


void AsioTimer::Op::Initiate(std::chrono::nanoseconds duration)
{
    _timer.expires_after(duration);
    _timer.async_wait([self = this->shared_from_this()](const asio::error_code& e) {
        self->OnAsioAsyncWaitComplete(e);
    });
}


void AsioTimer::Op::Shutdown()
{
    _timer.cancel();
}


void AsioTimer::Op::Abandon()
{
    _parent = nullptr;
}


void AsioTimer::Op::OnAsioAsyncWaitComplete(const asio::error_code& errorCode)
{
    if (errorCode)
    {
        return;
    }

    auto parent = _parent.load();
    if (parent == nullptr)
    {
        return;
    }

    parent->_listener->OnTimerExpired(*parent);
}


} // namespace VSilKit
