// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "VAsioPeerInfo.hpp"

#include "IRawByteStream.hpp"

#include <cstddef>
#include <chrono>
#include <memory>


namespace VSilKit {


struct IConnectPeerListener;


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
    virtual void OnConnectPeerFailure(IConnectPeer&, SilKit::Core::VAsioPeerInfo peerInfo) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IConnectPeer;
using VSilKit::IConnectPeerListener;
} // namespace Core
} // namespace SilKit
