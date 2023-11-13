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


#include "IConnectPeer.hpp"

#include "IIoContext.hpp"

#include "ILogger.hpp"

#include <chrono>
#include <functional>
#include <memory>


namespace SilKit {
namespace Core {
class VAsioConnection;
class VAsioPeer;
class Uri;
} // namespace Core
} // namespace SilKit


namespace VSilKit {


struct IConnectPeerListener;


class ConnectPeer
    : public IConnectPeer
    , private IConnectorListener
{
    using Uri = SilKit::Core::Uri;

    IIoContext* _ioContext{nullptr};
    SilKit::Services::Logging::ILogger* _logger{nullptr};
    SilKit::Core::VAsioPeerInfo _peerInfo;
    bool _enableDomainSockets{false};

    IConnectPeerListener* _listener{nullptr};

    size_t _remainingAttempts{1};
    size_t _uriIndex{0};
    std::vector<Uri> _uris;

    std::chrono::milliseconds _timeout{};

    std::unique_ptr<IConnector> _connector;

public:
    ConnectPeer(IIoContext* ioContext, SilKit::Services::Logging::ILogger* logger,
                const SilKit::Core::VAsioPeerInfo& peerInfo, bool enableDomainSockets);
    ~ConnectPeer() override;

public: // IConnectPeer
    void SetListener(IConnectPeerListener& listener) override;
    void AsyncConnect(size_t numberOfAttempts, std::chrono::milliseconds timeout) override;
    void Shutdown() override;

private:
    void UpdateUris();
    void TryNextUri();
    void HandleSuccess(std::unique_ptr<IRawByteStream> stream);
    void HandleFailure();

private: // IConnectorListener
    void OnAsyncConnectSuccess(IConnector&, std::unique_ptr<IRawByteStream> stream) override;
    void OnAsyncConnectFailure(IConnector&) override;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ConnectPeer;
} // namespace Core
} // namespace SilKit
