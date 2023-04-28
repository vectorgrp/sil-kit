/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "IVAsioConnectionPeer.hpp"
#include "IVAsioPeerConnection.hpp"

namespace SilKit {
namespace Services {
namespace Logging {
class ILogger;
} // namespace Logging
} // namespace Services
} // namespace SilKit


namespace SilKit {
namespace Core {

class VAsioConnection;

class VAsioProxyPeer
    : public IVAsioConnectionPeer
    , public IVAsioPeerConnection
{
public:
    VAsioProxyPeer(IVAsioPeerConnection* connection, VAsioPeerInfo peerInfo, IVAsioPeer* peer,
                   SilKit::Services::Logging::ILogger* logger);

public: // IVAsioPeer via IVAsioConnectionPeer
    void SendSilKitMsg(SerializedMessage buffer) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;
    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;
    auto GetRemoteAddress() const -> std::string override;
    auto GetLocalAddress() const -> std::string override;
    void StartAsyncRead() override;
    void DrainAllBuffers() override;
    void SetProtocolVersion(ProtocolVersion v) override;
    auto GetProtocolVersion() const -> ProtocolVersion override;

public: // IServiceEndpoint via IVAsioConnectionPeer
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override;

public: // IVAsioPeerConnection
    auto GetParticipantName() const -> const std::string& override;
    auto Config() const -> const SilKit::Config::ParticipantConfiguration& override;
    void OnSocketData(IVAsioPeer* from, SerializedMessage&& buffer) override;
    void OnPeerShutdown(IVAsioPeer* peer) override;

public:
    auto GetPeer() const -> IVAsioPeer*;

private:
    IVAsioPeerConnection* _connection;
    IVAsioPeer* _peer;
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    SilKit::Services::Logging::ILogger* _logger;
    ProtocolVersion _protocolVersion;
};

} // namespace Core
} // namespace SilKit
