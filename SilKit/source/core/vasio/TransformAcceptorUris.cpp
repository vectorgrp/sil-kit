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

#include "TransformAcceptorUris.hpp"

#include "Logger.hpp"

#include <asio/ip/address.hpp>

#include <set>

namespace SilKit {
namespace Core {

namespace {

using SilKit::Core::Uri;
using SilKit::SilKitError;

struct UriLexicographicLess
{
    bool operator()(const Uri& lhs, const Uri& rhs) const
    {
        return std::less<>{}(lhs.EncodedString(), rhs.EncodedString());
    }
};

} // namespace

inline auto GetUriInfo(const Uri& uri) -> UriInfo
{
    const auto host = [](std::string host) {
        size_t it;
        while ((it = host.find_first_of("[]")) != host.npos)
        {
            host.erase(it, 1);
        }
        return host;
    }(uri.Host());

    if (uri.Type() == Uri::UriType::Local)
    {
        UriInfo hostInfo;
        hostInfo.local = true;
        return hostInfo;
    }

    if (uri.Type() == Uri::UriType::Tcp)
    {
        try
        {
            const auto addr = asio::ip::make_address(host);

            UriInfo hostInfo;
            hostInfo.ip4 = addr.is_v4();
            hostInfo.ip6 = addr.is_v6();
            hostInfo.catchallIp = addr.is_unspecified();
            hostInfo.loopbackIp = addr.is_loopback();
            hostInfo.local = hostInfo.loopbackIp;
            return hostInfo;
        }
        catch (...)
        {
            // ignore any exceptions from parsing the address
            return UriInfo{};
        }
    }

    return UriInfo{};
}

auto TransformAcceptorUris(SilKit::Services::Logging::ILogger* logger, IVAsioPeer* advertisedPeer,
                           IVAsioPeer* audiencePeer) -> std::vector<std::string>
{
    const auto src = Uri::Parse(advertisedPeer->GetRemoteAddress());
    const auto srcInfo = GetUriInfo(src);

    const auto dst = Uri::Parse(audiencePeer->GetLocalAddress());
    const auto dstInfo = GetUriInfo(dst);

    std::set<Uri, UriLexicographicLess> acceptorUris;

    const auto acceptUri = [logger, &acceptorUris, audiencePeer, advertisedPeer](const Uri& uri) {
        Services::Logging::Debug(logger, "SIL Kit Registry: TransformAcceptorUris: '{}' to '{}': Accept: {}",
                                 advertisedPeer->GetInfo().participantName, audiencePeer->GetInfo().participantName,
                                 uri.EncodedString());
        acceptorUris.emplace(uri);
    };

    const auto acceptNewTcpUri = [logger, &acceptorUris, audiencePeer, advertisedPeer](const std::string& host,
                                                                                       uint16_t port) {
        auto uri = Uri::MakeTcp(host, port);
        Services::Logging::Debug(logger, "SIL Kit Registry: TransformAcceptorUris: '{}' to '{}': Accept: {}",
                                 advertisedPeer->GetInfo().participantName, audiencePeer->GetInfo().participantName,
                                 uri.EncodedString());
        acceptorUris.emplace(std::move(uri));
    };

    // sanity checks

    if (src.Type() != Uri::UriType::Local && src.Type() != Uri::UriType::Tcp)
    {
        throw SilKitError{
            "SIL Kit Registry: TransformAcceptorUris: Remote address of advertised peer has invalid UriType"};
    }

    if (dst.Type() != Uri::UriType::Local && dst.Type() != Uri::UriType::Tcp)
    {
        throw SilKitError{"SIL Kit Registry: TransformAcceptorUris: Local address of audience peer has invalid UriType"};
    }

    // URI transformation

    for (const auto& uriString : advertisedPeer->GetInfo().acceptorUris)
    {
        const auto uri = Uri::Parse(uriString);
        const auto uriInfo = GetUriInfo(uri);

        Services::Logging::Debug(logger, "SIL Kit Registry: TransformAcceptorUris: '{}' to '{}': Decide: {}",
                                 advertisedPeer->GetInfo().participantName, audiencePeer->GetInfo().participantName,
                                 uri.EncodedString());

        if (uriInfo.catchallIp)
        {
            if (srcInfo.local)
            {
                // The advertised participant connected locally (local-domain / tcp-loopback).

                if (dstInfo.local)
                {
                    // The audience participant connected locally (local-domain / tcp-loopback).

                    // NB: It may be beneficial to always send loopback acceptors when a catchall acceptor is processed
                    //     from a locally connected advertised participant.

                    if (uriInfo.ip4)
                    {
                        acceptNewTcpUri("127.0.0.1", uri.Port());
                    }

                    if (uriInfo.ip6)
                    {
                        acceptNewTcpUri("[::1]", uri.Port());
                    }
                }

                if (uriInfo.ip4 && dstInfo.ip4)
                {
                    acceptNewTcpUri(dst.Host(), uri.Port());
                }

                if (uriInfo.ip6 && dstInfo.ip6)
                {
                    acceptNewTcpUri(dst.Host(), uri.Port());
                }
            }

            if (uriInfo.ip4 && srcInfo.ip4)
            {
                acceptNewTcpUri(src.Host(), uri.Port());
            }

            if (uriInfo.ip6 && srcInfo.ip6)
            {
                acceptNewTcpUri(src.Host(), uri.Port());
            }

            // Never forward a catchall acceptor, they cannot be handled properly by any audience participant.
            continue;
        }

        acceptUri(uri);
    }

    // Order the acceptor URIs before sending them to the audience participant. Local-domain acceptors always have the
    // highest priority.

    std::multimap<int, std::string> orderedAcceptorUris;

    // The key of the multimap is a penalty, lower values are placed earlier in the final list of acceptors.
    // The default penalties are set for a non-local audience (to keep the if-condition positive).

    // If the audience is connecting from a non-local address (neither local-domain, nor tcp-loopback), then send
    // tcp-loopback acceptors after non-local acceptors.

    int localDomainPenalty = 0, nonLocalPenalty = 1000, loopbackPenalty = 2000;

    if (dstInfo.local)
    {
        // If the audience is connecting from a local address (local-domain or tcp-loopback), then send tcp-loopback
        // acceptors before non-local acceptors.
        loopbackPenalty = 1000;
        nonLocalPenalty = 2000;
    }

    for (const auto& uri : acceptorUris)
    {
        if (uri.Type() == Uri::UriType::Local)
        {
            orderedAcceptorUris.emplace(localDomainPenalty, uri.EncodedString());
            continue;
        }

        if (GetUriInfo(uri).loopbackIp)
        {
            orderedAcceptorUris.emplace(loopbackPenalty, uri.EncodedString());
            continue;
        }

        orderedAcceptorUris.emplace(nonLocalPenalty, uri.EncodedString());
    }

    std::vector<std::string> resultingAcceptorUris;

    for (const auto& pair : orderedAcceptorUris)
    {
        resultingAcceptorUris.emplace_back(pair.second);
    }

    return resultingAcceptorUris;
}

} // namespace Core
} // namespace SilKit
