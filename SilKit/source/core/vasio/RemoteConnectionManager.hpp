// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "ConnectPeer.hpp"
#include "ITimer.hpp"

#include <mutex>
#include <vector>


namespace SilKit {
namespace Core {
class VAsioConnection;
} // namespace Core
} // namespace SilKit


namespace SilKit {
namespace Core {


struct RemoteConnectionManagerSettings
{
    std::chrono::milliseconds connectTimeout{5000};
};


class RemoteConnectionManager : IConnectPeerListener
{
    VAsioConnection* _vAsioConnection{nullptr};
    RemoteConnectionManagerSettings _settings;

    std::mutex _mutex;
    std::vector<std::unique_ptr<IConnectPeer>> _connectPeers;

public:
    explicit RemoteConnectionManager(VAsioConnection& vAsioConnection, const RemoteConnectionManagerSettings& settings);

    void StartConnectingTo(const VAsioPeerInfo& peerInfo);
    void Shutdown();

private:
    void Remove(const IConnectPeer& connectPeer);

private: // IConnectPeerListener
    void OnConnectPeerSuccess(IConnectPeer& connectPeer, VAsioPeerInfo peerInfo,
                              std::unique_ptr<IRawByteStream> stream) override;
    void OnConnectPeerFailure(IConnectPeer& connectPeer, VAsioPeerInfo peerInfo) override;
};


} // namespace Core
} // namespace SilKit
