// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Uri.hpp"

#include "ib/exception.hpp"

namespace ib {
namespace mw {

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
    return _port;
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
    uri << "vib://" << host << ":" << port;
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
        throw ib::ConfigurationError("Uri::Parse: could not find scheme "
            "separator in user input: \"" + rawUri + "\"");
    }
    uri._scheme = rawUri.substr(0, idx);
    rawUri = rawUri.substr(idx + schemeSeparator.size());

    //legacy configuration of 'local:///domainsocketpath' and 'tcp://localhost' URIs
    if(uri.Scheme() == "tcp")
    {
        uri.SetType(UriType::Tcp);
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
        if(idx != rawUri.npos)
        {
            uri._host = rawUri.substr(0, idx);
            std::stringstream  portStr;
            portStr << rawUri.substr(idx+1);
            portStr >> uri._port; //parse string to uint16_t
            if(portStr.fail())
            {
                throw ib::ConfigurationError("Uri::Parse: failed to parse the port "
                    "number: " + portStr.str());
            }

            if (uri._host.empty() || portStr.str().empty())
            {
                throw ib::ConfigurationError("Uri::Parse: URI contains no host or port");
            }
        }
    }
    return uri;
}

} // namespace mw
} // namespace ib
