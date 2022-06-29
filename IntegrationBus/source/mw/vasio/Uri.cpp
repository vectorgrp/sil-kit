// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "Uri.hpp"

namespace {
const std::string localPrefix{ "local://" };
const std::string tcpPrefix{ "tcp://" };
} // namespace

namespace ib {
namespace mw {

auto Uri::EncodedString() const -> const std::string&
{
    return _uriString;
}

auto Uri::Host() const -> const std::string&
{
    if (_host.empty())
    {
        throw std::logic_error{ "Uri::Host(): must not be empty: uriString= " + _uriString };
    }
    return _host;
}

auto Uri::Port() const -> uint16_t
{
    return _port;
}

auto Uri::Path() const -> const std::string&
{
    return _path;
}

auto Uri::Type() const -> UriType
{
    return _type;
}

Uri::Uri(const std::string& uriStr)
{
    *this = Uri::parse(uriStr);
}

Uri::Uri(const asio::local::stream_protocol::endpoint& ep)
{
    std::stringstream uri;
    uri << localPrefix << ep.path();
    *this = Uri::parse(uri.str());
}

Uri::Uri(const asio::ip::tcp::endpoint& ep)
{
    std::stringstream uri;
    uri << tcpPrefix << ep; //will be "ipv4:port" or "[ipv6]:port"
    *this = Uri::parse(uri.str());
}

Uri::Uri(const std::string& host, const uint16_t port)
{
    std::stringstream uri;
    uri << tcpPrefix << host << ":" << port;
    *this = Uri::parse(uri.str());
}

auto Uri::parse(const std::string& uriRepr) -> Uri
{
    Uri uri{};
    uri._uriString = uriRepr;

    if (uriRepr.find(tcpPrefix) == 0)
    {
        const auto hostAndPort = uriRepr.substr(tcpPrefix.size());
        auto sep = hostAndPort.rfind(":");
        if (sep == hostAndPort.npos)
        {
            throw std::runtime_error{ "Uri: invalid tcp:// URI: missing colon" };
        }
        uri._host = hostAndPort.substr(0, sep);
        std::stringstream portStr;
        portStr << hostAndPort.substr(sep + 1);
        if (uri._host.empty() || portStr.str().empty())
        {
            throw std::runtime_error{ "Uri: invalid host or port for tcp://" };
        }
        portStr >> uri._port;
        uri._type = UriType::Tcp;
    }
    else if (uriRepr.find(localPrefix) == 0)
    {
        const auto rawPath = uriRepr.substr(localPrefix.size());
        uri._path = rawPath;
        if (uri._path.empty())
        {
            throw std::runtime_error{ "Uri: invalid path for \"local://\": \"" + uriRepr +"\"" };
        }
        uri._type = UriType::Local;
    }
    else
    {
        throw std::runtime_error{"Uri: unknown URI protocol specified: " + uriRepr};
    }
    return uri;
}

} // namespace mw
} // namespace ib
