// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
