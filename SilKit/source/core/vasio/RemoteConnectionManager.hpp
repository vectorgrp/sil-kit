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
