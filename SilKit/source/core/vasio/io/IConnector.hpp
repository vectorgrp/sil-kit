// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IRawByteStream.hpp"

#include <chrono>
#include <memory>


namespace VSilKit {


struct IConnectorListener;


struct IConnector
{
    virtual ~IConnector() = default;

    virtual void SetListener(IConnectorListener& listener) = 0;

    virtual void AsyncConnect(std::chrono::milliseconds timeout) = 0;

    virtual void Shutdown() = 0;
};


struct IConnectorListener
{
    virtual ~IConnectorListener() = default;

    virtual void OnAsyncConnectSuccess(IConnector& connector, std::unique_ptr<IRawByteStream> stream) = 0;

    virtual void OnAsyncConnectFailure(IConnector& connector) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IConnector;
using VSilKit::IConnectorListener;
} // namespace Core
} // namespace SilKit
