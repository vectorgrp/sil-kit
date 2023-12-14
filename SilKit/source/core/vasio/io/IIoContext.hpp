// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IAcceptor.hpp"
#include "IConnector.hpp"
#include "ITimer.hpp"

#include "ILogger.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace VSilKit {


struct IIoContext
{
    virtual ~IIoContext() = default;

    virtual void Run() = 0;

    virtual void Post(std::function<void()> function) = 0;

    virtual void Dispatch(std::function<void()> function) = 0;

    virtual auto MakeTcpAcceptor(const std::string& address, uint16_t port) -> std::unique_ptr<IAcceptor> = 0;

    virtual auto MakeLocalAcceptor(const std::string& path) -> std::unique_ptr<IAcceptor> = 0;

    virtual auto MakeTcpConnector(const std::string& address, uint16_t port) -> std::unique_ptr<IConnector> = 0;

    virtual auto MakeLocalConnector(const std::string& path) -> std::unique_ptr<IConnector> = 0;

    virtual auto MakeTimer() -> std::unique_ptr<ITimer> = 0;

    virtual auto Resolve(const std::string& name) -> std::vector<std::string> = 0;

    virtual void SetLogger(SilKit::Services::Logging::ILogger& logger) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IIoContext;
} // namespace Core
} // namespace SilKit
