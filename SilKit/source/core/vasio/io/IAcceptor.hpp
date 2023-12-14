// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IRawByteStream.hpp"

#include <chrono>
#include <memory>


namespace VSilKit {


struct IAcceptorListener;


struct IAcceptor
{
    virtual ~IAcceptor() = default;

    virtual void SetListener(IAcceptorListener& listener) = 0;

    virtual auto GetLocalEndpoint() const -> std::string = 0;

    virtual void AsyncAccept(std::chrono::milliseconds timeout) = 0;

    virtual void Shutdown() = 0;
};


struct IAcceptorListener
{
    virtual ~IAcceptorListener() = default;

    virtual void OnAsyncAcceptSuccess(IAcceptor& acceptor, std::unique_ptr<IRawByteStream> stream) = 0;

    virtual void OnAsyncAcceptFailure(IAcceptor& acceptor) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IAcceptor;
using VSilKit::IAcceptorListener;
} // namespace Core
} // namespace SilKit
