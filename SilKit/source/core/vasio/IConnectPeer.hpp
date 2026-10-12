// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "core/vasio/VAsioPeerInfo.hpp"

#include "core/vasio/io/IRawByteStream.hpp"

#include <cstddef>
#include <chrono>
#include <memory>
#include <string>
#include <system_error>
#include <vector>


namespace VSilKit {


struct IConnectPeerListener;


/// Why connecting to one of the acceptor URIs of a peer failed
struct ConnectPeerFailure
{
    enum class Stage
    {
        Resolve, //!< the host name of a tcp:// URI could not be resolved
        Connect, //!< the connection attempt failed, see errorCode
        Other,   //!< the URI could not be used, e.g., local-domain sockets are disabled
    };

    std::string uri;
    bool isLocal{false};
    Stage stage{Stage::Other};
    std::error_code errorCode;
    std::string message;
};

using ConnectPeerFailures = std::vector<ConnectPeerFailure>;

/// Formats the failures as "<uri>: <message>", separated by "; "
inline auto FormatConnectPeerFailures(const ConnectPeerFailures& failures) -> std::string
{
    std::string result;
    for (const auto& failure : failures)
    {
        if (!result.empty())
        {
            result += "; ";
        }
        result += failure.uri + ": " + failure.message;
    }
    return result;
}


struct IConnectPeer
{
    virtual ~IConnectPeer() = default;

    virtual void SetListener(IConnectPeerListener& listener) = 0;
    virtual void AsyncConnect(size_t numberOfAttempts, std::chrono::milliseconds timeout) = 0;
    virtual void Shutdown() = 0;
};


struct IConnectPeerListener
{
    virtual ~IConnectPeerListener() = default;

    virtual void OnConnectPeerSuccess(IConnectPeer&, SilKit::Core::VAsioPeerInfo peerInfo,
                                      std::unique_ptr<IRawByteStream> stream) = 0;
    virtual void OnConnectPeerFailure(IConnectPeer&, SilKit::Core::VAsioPeerInfo peerInfo,
                                      const ConnectPeerFailures& failures) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ConnectPeerFailure;
using VSilKit::ConnectPeerFailures;
using VSilKit::IConnectPeer;
using VSilKit::IConnectPeerListener;
} // namespace Core
} // namespace SilKit
