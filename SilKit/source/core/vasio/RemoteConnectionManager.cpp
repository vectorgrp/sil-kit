// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "RemoteConnectionManager.hpp"

#include "VAsioConnection.hpp"
#include "VAsioPeer.hpp"
#include "ConnectPeer.hpp"
#include "util/TracingMacros.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioConnection
#define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#define SILKIT_TRACE_METHOD_(...)
#endif


using namespace std::chrono_literals;

namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {


RemoteConnectionManager::RemoteConnectionManager(VAsioConnection& vAsioConnection,
                                                 const RemoteConnectionManagerSettings& settings)
    : _vAsioConnection{&vAsioConnection}
    , _settings{settings}
{
}


void RemoteConnectionManager::StartConnectingTo(const VAsioPeerInfo& peerInfo)
{
    auto connectPeerPtr{_vAsioConnection->MakeConnectPeer(peerInfo)};
    connectPeerPtr->SetListener(*this);

    auto& connectPeer = *connectPeerPtr;
    _connectPeers.emplace_back(std::move(connectPeerPtr));

    connectPeer.AsyncConnect(1, _settings.connectTimeout);
}


void RemoteConnectionManager::Shutdown()
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    for (const auto& connectPeer : _connectPeers)
    {
        connectPeer->Shutdown();
    }
}


void RemoteConnectionManager::Remove(const IConnectPeer& connectPeer)
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    _connectPeers.erase(std::remove_if(_connectPeers.begin(), _connectPeers.end(),
                                       [needle = &connectPeer](const auto& hay) { return needle == hay.get(); }));
}


void RemoteConnectionManager::OnConnectPeerSuccess(IConnectPeer& connectPeer, VAsioPeerInfo peerInfo,
                                                   std::unique_ptr<IRawByteStream> stream)
{
    SilKit::Services::Logging::Debug(_vAsioConnection->_logger,
                                     "Successfully connected to {} after receiving a remote connect request",
                                     peerInfo.participantName);

    Remove(connectPeer);

    auto vAsioPeer{_vAsioConnection->MakeVAsioPeer(std::move(stream))};
    vAsioPeer->SetInfo(std::move(peerInfo));

    _vAsioConnection->OnRemoteConnectionSuccess(std::move(vAsioPeer));
}

void RemoteConnectionManager::OnConnectPeerFailure(IConnectPeer& connectPeer, VAsioPeerInfo peerInfo)
{
    SilKit::Services::Logging::Debug(_vAsioConnection->_logger,
                                     "Failed to connect to {} after receiving a remote connect request",
                                     peerInfo.participantName);

    Remove(connectPeer);
    _vAsioConnection->OnRemoteConnectionFailure(std::move(peerInfo));
}


} // namespace Core
} // namespace SilKit


#undef SILKIT_TRACE_METHOD_
