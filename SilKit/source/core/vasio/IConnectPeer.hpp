// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
