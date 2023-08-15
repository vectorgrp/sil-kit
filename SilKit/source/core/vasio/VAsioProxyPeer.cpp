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

#include "VAsioProxyPeer.hpp"

#include "Logger.hpp"


namespace {
using SilKit::Services::Logging::Debug;
using SilKit::Services::Logging::Trace;
} // namespace


namespace SilKit {
namespace Core {

VAsioProxyPeer::VAsioProxyPeer(IVAsioPeerConnection *connection, VAsioPeerInfo peerInfo, IVAsioPeer *peer,
                               SilKit::Services::Logging::ILogger *logger)
    : _connection{connection}
    , _peer{peer}
    , _peerInfo{std::move(peerInfo)}
    , _logger{logger}
{
    Debug(_logger, "VAsioProxyPeer ({}): Created with proxy {}", _peerInfo.participantName,
          _peer->GetInfo().participantName);
}

// ================================================================================
//  IVAsioPeer via IVAsioConnectionPeer
// ================================================================================

void VAsioProxyPeer::SendSilKitMsg(SerializedMessage buffer)
{
    ProxyMessage msg{};
    msg.source = GetParticipantName();
    msg.destination = GetInfo().participantName;
    msg.payload = buffer.ReleaseStorage();

    Trace(_logger, "VAsioProxyPeer ({}): SendSilKitMsg({})", _peerInfo.participantName, msg.payload.size());

    _peer->SendSilKitMsg(SerializedMessage{msg});
}

void VAsioProxyPeer::Subscribe(VAsioMsgSubscriber subscriber)
{
    Services::Logging::Debug(_logger,
                             "VAsioProxyPeer: Subscribing to messages of type '{}' on link '{}' from participant '{}'",
                             subscriber.msgTypeName, subscriber.networkName, _peerInfo.participantName);

    SendSilKitMsg(SerializedMessage{subscriber});
}

auto VAsioProxyPeer::GetInfo() const -> const VAsioPeerInfo &
{
    return _peerInfo;
}

void VAsioProxyPeer::SetInfo(VAsioPeerInfo info)
{
    _peerInfo = std::move(info);
}

auto VAsioProxyPeer::GetRemoteAddress() const -> std::string
{
    return _peer->GetRemoteAddress();
}

auto VAsioProxyPeer::GetLocalAddress() const -> std::string
{
    return _peer->GetLocalAddress();
}

void VAsioProxyPeer::StartAsyncRead()
{
    Debug(_logger, "VAsioProxyPeer ({}): StartAsyncRead: Ignored", _peerInfo.participantName);
}

void VAsioProxyPeer::DrainAllBuffers()
{
    Debug(_logger, "VAsioProxyPeer ({}): DrainAllBuffers: Ignored", _peerInfo.participantName);
}

void VAsioProxyPeer::SetProtocolVersion(ProtocolVersion v)
{
    Debug(_logger, "VAsioProxyPeer ({}): SetProtocolVersion: {}.{}", _peerInfo.participantName, v.major, v.minor);
    _protocolVersion = v;
}

auto VAsioProxyPeer::GetProtocolVersion() const -> ProtocolVersion
{
    Trace(_logger, "VAsioProxyPeer ({}): GetProtocolVersion: {}.{}", _peerInfo.participantName, _protocolVersion.major, _protocolVersion.minor);
    return _protocolVersion;
}

// ================================================================================
//  IServiceEndpoint via IVAsioConnectionPeer
// ================================================================================

void VAsioProxyPeer::SetServiceDescriptor(ServiceDescriptor const &serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto VAsioProxyPeer::GetServiceDescriptor() const -> ServiceDescriptor const &
{
    return _serviceDescriptor;
}

// ================================================================================
//  IVAsioPeerConnection
// ================================================================================

auto VAsioProxyPeer::GetParticipantName() const -> const std::string &
{
    return _connection->GetParticipantName();
}

auto VAsioProxyPeer::Config() const -> const SilKit::Config::ParticipantConfiguration &
{
    return _connection->Config();
}

void VAsioProxyPeer::OnSocketData(IVAsioPeer *from, SerializedMessage &&buffer)
{
    _connection->OnSocketData(from, std::move(buffer));
}

void VAsioProxyPeer::OnPeerShutdown(IVAsioPeer *peer)
{
    _connection->OnPeerShutdown(peer);
}

auto VAsioProxyPeer::GetPeer() const -> IVAsioPeer *
{
    return _peer;
}

} // namespace Core
} // namespace SilKit
