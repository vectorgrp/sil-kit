// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IConnectPeer.hpp"

#include "IIoContext.hpp"

#include "LoggerMessage.hpp"

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
