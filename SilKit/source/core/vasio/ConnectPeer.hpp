// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "core/vasio/IConnectPeer.hpp"

#include "core/vasio/io/IIoContext.hpp"

#include "services/logging/LoggerMessage.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>


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
    SilKit::Services::Logging::ILoggerInternal* _logger{nullptr};
    SilKit::Core::VAsioPeerInfo _peerInfo;
    bool _enableDomainSockets{false};

    IConnectPeerListener* _listener{nullptr};

    size_t _remainingAttempts{1};
    size_t _uriIndex{0};
    std::vector<Uri> _uris;

    std::chrono::milliseconds _timeout{};

    std::unique_ptr<IConnector> _connector;

    /// The most recent failure reason for each URI, in the order the URIs failed first
    std::vector<std::pair<std::string, std::string>> _failureReasons;

public:
    ConnectPeer(IIoContext* ioContext, SilKit::Services::Logging::ILoggerInternal* logger,
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
    void RecordFailure(const std::string& uri, std::string reason);

private: // IConnectorListener
    void OnAsyncConnectSuccess(IConnector&, std::unique_ptr<IRawByteStream> stream) override;
    void OnAsyncConnectFailure(IConnector&, std::error_code errorCode) override;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ConnectPeer;
} // namespace Core
} // namespace SilKit
