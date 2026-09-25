// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/io/impl/SetAsioSocketOptions.hpp"

#include "services/logging/LoggerMessage.hpp"


namespace {
namespace Log = SilKit::Services::Logging;
} // namespace


namespace VSilKit {


void SetAsioSocketOptions(Log::ILoggerInternal* logger, asio::ip::tcp::socket& socket,
                          const AsioSocketOptions& socketOptions, std::error_code& errorCode)
{
    if (socketOptions.tcp.noDelay)
    {
        socket.set_option(asio::ip::tcp::no_delay{true}, errorCode);
        if (errorCode)
        {
            logger->MakeMessage(SilKit::Services::Logging::Level::Warn, SilKit::Services::Logging::Topic::Asio)
                .SetMessage("SetAsioSocketOptions: failed to enable 'no delay' option")
                .Dispatch();
            return;
        }
    }

    if (socketOptions.tcp.receiveBufferSize > 0)
    {
        socket.set_option(asio::socket_base::receive_buffer_size{socketOptions.tcp.receiveBufferSize}, errorCode);
        if (errorCode)
        {
            logger->MakeMessage(SilKit::Services::Logging::Level::Warn, SilKit::Services::Logging::Topic::Asio)
                .SetMessage("SetAsioSocketOptions: failed to set receive buffer size to {}: {}",
                            socketOptions.tcp.receiveBufferSize, errorCode.message())
                .Dispatch();
            return;
        }
    }

    if (socketOptions.tcp.sendBufferSize > 0)
    {
        socket.set_option(asio::socket_base::send_buffer_size{socketOptions.tcp.sendBufferSize}, errorCode);
        if (errorCode)
        {

            logger->MakeMessage(SilKit::Services::Logging::Level::Warn, SilKit::Services::Logging::Topic::Asio)
                .SetMessage("SetAsioSocketOptions: failed to set send buffer size to {}: {}",
                            socketOptions.tcp.sendBufferSize, errorCode.message())
                .Dispatch();
            return;
        }
    }

#if defined(TCP_NOTSENT_LOWAT)
    if (socketOptions.tcp.notSentLowWatermark > 0)
    {
        using NotSentLowWatermark = asio::detail::socket_option::integer<IPPROTO_TCP, TCP_NOTSENT_LOWAT>;
        socket.set_option(NotSentLowWatermark{socketOptions.tcp.notSentLowWatermark}, errorCode);
        if (errorCode)
        {
            logger->MakeMessage(SilKit::Services::Logging::Level::Warn, SilKit::Services::Logging::Topic::Asio)
                .SetMessage("SetAsioSocketOptions: failed to set not sent low watermark to {}: {}",
                            socketOptions.tcp.notSentLowWatermark, errorCode.message())
                .Dispatch();
            return;
        }
    }
#endif
}


void SetAsioSocketOptions(SilKit::Services::Logging::ILoggerInternal*, asio::local::stream_protocol::socket&,
                          const AsioSocketOptions&, std::error_code&)
{
    // no local-domain specific options
}


} // namespace VSilKit
