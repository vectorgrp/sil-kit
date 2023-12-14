// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "AsioFormatEndpoint.hpp"

#include "util/Exceptions.hpp"

#include <sstream>


namespace VSilKit {


auto FormatEndpoint(const asio::ip::tcp::endpoint& ep) -> std::string
{
    std::ostringstream ss;
    ss << "tcp://" << ep;
    return ss.str();
}


auto FormatEndpoint(const asio::local::stream_protocol::endpoint& ep) -> std::string
{
    std::ostringstream ss;
    ss << "local://" << ep.path();
    return ss.str();
}


auto FormatEndpoint(const asio::generic::stream_protocol::endpoint& ep) -> std::string
{
    const auto family{ep.protocol().family()};

    if (family == asio::ip::tcp::v4().family() || family == asio::ip::tcp::v6().family())
    {
        return FormatEndpoint(reinterpret_cast<const asio::ip::tcp::endpoint&>(ep));
    }
    else if (family == asio::local::stream_protocol().family())
    {
        return FormatEndpoint(asio::local::stream_protocol::endpoint{ep.data()->sa_data});
    }
    else
    {
        throw InvalidAsioEndpointProtocolFamily{};
    }
}


} // namespace VSilKit
