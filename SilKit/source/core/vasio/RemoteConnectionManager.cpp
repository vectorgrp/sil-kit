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


#include "RemoteConnectionManager.hpp"

#include "VAsioConnection.hpp"
#include "VAsioPeer.hpp"
#include "ConnectPeer.hpp"
#include "util/TracingMacros.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_VAsioConnection
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


using namespace std::chrono_literals;

namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {


RemoteConnectionManager::RemoteConnectionManager(VAsioConnection &vAsioConnection,
                                                 const RemoteConnectionManagerSettings &settings)
    : _vAsioConnection{&vAsioConnection}
    , _settings{settings}
{
}


void RemoteConnectionManager::StartConnectingTo(const VAsioPeerInfo &peerInfo)
{
    auto connectPeerPtr{_vAsioConnection->MakeConnectPeer(peerInfo)};
    connectPeerPtr->SetListener(*this);

    auto &connectPeer = *connectPeerPtr;
    _connectPeers.emplace_back(std::move(connectPeerPtr));

    connectPeer.AsyncConnect(1, _settings.connectTimeout);
}


void RemoteConnectionManager::Shutdown()
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    for (const auto &connectPeer : _connectPeers)
    {
        connectPeer->Shutdown();
    }
}


void RemoteConnectionManager::Remove(const IConnectPeer &connectPeer)
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    _connectPeers.erase(
        std::remove_if(_connectPeers.begin(), _connectPeers.end(), [needle = &connectPeer](const auto &hay) {
            return needle == hay.get();
        }));
}


void RemoteConnectionManager::OnConnectPeerSuccess(IConnectPeer &connectPeer, VAsioPeerInfo peerInfo,
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

void RemoteConnectionManager::OnConnectPeerFailure(IConnectPeer &connectPeer, VAsioPeerInfo peerInfo)
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
