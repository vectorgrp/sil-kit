/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "Uri.hpp"

#include "silkit/participant/exception.hpp"

#include "fmt/format.h"

namespace SilKit {
namespace Core {

auto Uri::EncodedString() const -> const std::string&
{
    return _uriString;
}

auto Uri::Host() const -> const std::string&
{
    return _host;
}

auto Uri::Port() const -> uint16_t
{
    if(_port)
    {
        return *_port;
    }
    //return default value if not set
    if(Type() == UriType::Local)
    {
        return 0;
    }
    else
    {
        return 8500;
    }
}

auto Uri::Scheme() const -> const std::string&
{
    return _scheme;
}

auto Uri::Path() const -> const std::string&
{
    return _path;
}

auto Uri::Type() const -> UriType
{
    return _type;
}


void Uri::SetType(UriType newType)
{
    _type = newType;
}

Uri::Uri(const std::string& uriStr)
{
    *this = Uri::Parse(uriStr);
}

Uri::Uri(const std::string& host, const uint16_t port)
{
    std::stringstream uri;
    uri << "silkit://" << host << ":" << port;
    *this = Uri::Parse(uri.str());
}

auto Uri::Parse(std::string rawUri) -> Uri
{
    const std::string schemeSeparator = "://";
    const std::string pathSeparator = "/";
    const std::string portSeparator = ":";
    Uri uri{};
    uri._uriString = rawUri;

    auto idx = rawUri.find(schemeSeparator);
    if(idx == rawUri.npos)
    {
        throw SilKit::ConfigurationError("Uri::Parse: could not find scheme "
            "separator in user input: \"" + rawUri + "\"");
    }
    uri._scheme = rawUri.substr(0, idx);
    rawUri = rawUri.substr(idx + schemeSeparator.size());

    //legacy configuration of 'local:///domainsocketpath' and 'tcp://localhost' URIs
    if(uri.Scheme() == "tcp")
    {
        uri.SetType(UriType::Tcp);
    }
    else if(uri.Scheme() == "silkit")
    {
        uri.SetType(UriType::Tcp); //we default to TCP streams
    }
    else if(uri.Scheme() == "local")
    {
        uri.SetType(UriType::Local);
    }
   
    if(uri.Type() == UriType::Local)
    {
        //must be a path, might contain ':' (currently not quoted)
        uri._path = rawUri;
    }
    else
    {
    // find network and path separator in 'hostname:port/path/foo/bar'
        idx = rawUri.find(pathSeparator);
        if(idx != rawUri.npos)
        {
            //we have trailing '/path;params?query'
            uri._path = rawUri.substr(idx+1);
            rawUri = rawUri.substr(0, idx);
        }
        //find port, if any in 'hostnameOrIp:port'
        idx = rawUri.rfind(portSeparator);
        uri._host = rawUri.substr(0, idx);
        if (idx != rawUri.npos)
        {
            std::stringstream  portStr;
            uint16_t port;
            portStr << rawUri.substr(idx+1);
            portStr >> port; //parse string to uint16_t
            if(portStr.fail())
            {
                throw SilKit::ConfigurationError("Uri::Parse: failed to parse the port "
                    "number: " + portStr.str());
            }

            if (portStr.str().empty())
            {
                throw SilKit::ConfigurationError("Uri::Parse: URI with port separator contains no port");
            }
            uri._port = port;

        }
        if (uri._host.empty())
        {
            throw SilKit::ConfigurationError("Uri::Parse: URI has empty host field");
        }
    }
    return uri;
}

auto Uri::MakeTcp(const std::string& host, const uint16_t port) -> Uri
{
    return Uri::Parse(fmt::format("tcp://{}:{}", host, port));
}

auto Uri::UrlEncode(const std::string& name) -> std::string
{
    std::string safeName;
    for (const uint8_t ch : name)
    {
        if (std::isalnum(static_cast<int>(ch)))
        {
            safeName.push_back(ch);
        }
        else
        {
            safeName += fmt::format("%{:02x}", static_cast<unsigned char>(ch));
        }
    }
    return safeName;
}

} // namespace Core
} // namespace SilKit
