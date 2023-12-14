// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SetAsioSocketOptions.hpp"

#include "ILogger.hpp"


namespace {
namespace Log = SilKit::Services::Logging;
} // namespace


namespace VSilKit {


void SetAsioSocketOptions(SilKit::Services::Logging::ILogger* logger, asio::ip::tcp::socket& socket,
                      const AsioSocketOptions& socketOptions, std::error_code& errorCode)
{
    if (socketOptions.tcp.noDelay)
    {
        socket.set_option(asio::ip::tcp::no_delay{true}, errorCode);
        if (errorCode)
        {
            Log::Warn(logger, "SetAsioSocketOptions: failed to enable 'no delay' option");
            return;
        }
    }

    if (socketOptions.tcp.receiveBufferSize > 0)
    {
        socket.set_option(asio::socket_base::receive_buffer_size{socketOptions.tcp.receiveBufferSize}, errorCode);
        if (errorCode)
        {
            Log::Warn(logger, "SetAsioSocketOptions: failed to set receive buffer size to {}: {}",
                      socketOptions.tcp.receiveBufferSize, errorCode.message());
            return;
        }
    }

    if (socketOptions.tcp.sendBufferSize > 0)
    {
        socket.set_option(asio::socket_base::send_buffer_size{socketOptions.tcp.sendBufferSize}, errorCode);
        if (errorCode)
        {
            Log::Warn(logger, "SetAsioSocketOptions: failed to set send buffer size to {}: {}",
                      socketOptions.tcp.sendBufferSize, errorCode.message());
            return;
        }
    }
}


void SetAsioSocketOptions(SilKit::Services::Logging::ILogger*, asio::local::stream_protocol::socket&,
                      const AsioSocketOptions&, std::error_code&)
{
    // no local-domain specific options
}


} // namespace VSilKit
