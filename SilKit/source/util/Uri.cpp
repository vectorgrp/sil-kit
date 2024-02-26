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


namespace {

auto ValidateAndTransformSimulationName(const std::string& simulationName) -> std::string {
    std::ostringstream ss;

    for (size_t pos = 0; pos != simulationName.size(); ++pos)
    {
        const bool last{pos == simulationName.size() - 1};
        const char ch{simulationName[pos]};

        if (ch == '/')
        {
            if (!last && simulationName[pos + 1] == '/')
            {
                throw SilKit::SilKitError{"simulation name may not contain multiple consecutive slashes"};
            }

            // the 'root' path (i.e., /) corresponds to the 'default' simulation name (which is empty)
            if (pos == 0)
            {
                continue;
            }
        }
        else
        {
            const bool isLetter{('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')};
            const bool isDigit{('0' <= ch && ch <= '9')};
            const bool isSymbol{ch == '_' || ch == '-' || ch == '.' || ch == '~'};

            if (!(isLetter || isDigit || isSymbol))
            {
                throw SilKit::SilKitError{
                    "simulation name may only contain a-z, A-Z, 0-9, _, -, ., ~, and / characters"};
            }
        }

        ss << ch;
    }

    return ss.str();
}

} // namespace


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
        uri.SetType(UriType::SilKit); //we default to TCP streams
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
            uri._path = ValidateAndTransformSimulationName(rawUri.substr(idx));
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

auto Uri::MakeSilKit(const std::string& host, const uint16_t port, const std::string& simulationName) -> Uri
{
    return Uri::Parse(fmt::format("silkit://{}:{}/{}", host, port, ValidateAndTransformSimulationName(simulationName)));
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


auto operator<<(std::ostream& ostream, UriType uriType) -> std::ostream&
{
    switch (uriType)
    {
    case UriType::Undefined:
        return ostream << "UriType::Undefined";
    case UriType::SilKit:
        return ostream << "UriType::SilKit";
    case UriType::Tcp:
        return ostream << "UriType::Tcp";
    case UriType::Local:
        return ostream << "UriType::Local";
    default:
        return ostream << "UriType(" << static_cast<std::underlying_type_t<UriType>>(uriType) << ")";
    }
}


} // namespace Core
} // namespace SilKit
