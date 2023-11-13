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

#include "IConnectPeer.hpp"
#include "IVAsioPeer.hpp"

#include <cstddef>
#include <chrono>
#include <memory>


namespace VSilKit {


/// This interface owns the methods required for the ConnectKnownParticipants component.
/// It should be broken up further in the future, since it combines
/// - the factory functions for IConnectPeer and IVAsioPeer objects,
/// - the networking function HandleConnectedPeer, TryRemoteConnectRequest, and TryProxyConnect,
/// - and the peer management function AddPeer.
struct IConnectionMethods
{
    virtual ~IConnectionMethods() = default;

    virtual auto MakeConnectPeer(const SilKit::Core::VAsioPeerInfo& peerInfo) -> std::unique_ptr<IConnectPeer> = 0;
    virtual auto MakeVAsioPeer(std::unique_ptr<IRawByteStream> stream) -> std::unique_ptr<SilKit::Core::IVAsioPeer> = 0;

    virtual void HandleConnectedPeer(SilKit::Core::IVAsioPeer* peer) = 0;
    virtual void AddPeer(std::unique_ptr<SilKit::Core::IVAsioPeer> peer) = 0;

    virtual auto TryRemoteConnectRequest(const SilKit::Core::VAsioPeerInfo& peerInfo) -> bool = 0;
    virtual auto TryProxyConnect(const SilKit::Core::VAsioPeerInfo& peerInfo) -> bool = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IConnectionMethods;
} // namespace Core
} // namespace SilKit
