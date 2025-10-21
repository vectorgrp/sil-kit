// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "VAsioProxyPeer.hpp"

#include "Logger.hpp"


namespace {
namespace Log = SilKit::Services::Logging;
} // namespace


namespace SilKit {
namespace Core {


VAsioProxyPeer::VAsioProxyPeer(IVAsioPeerListener* listener, std::string participantName, VAsioPeerInfo peerInfo,
                               IVAsioPeer* peer, SilKit::Services::Logging::ILogger* logger)
    : _listener{listener}
    , _participantName{std::move(participantName)}
    , _peer{peer}
    , _peerInfo{std::move(peerInfo)}
    , _logger{logger}
{
    Log::Debug(_logger, "VAsioProxyPeer ({}): Created with proxy {}", _peerInfo.participantName,
               _peer->GetInfo().participantName);
}

// ================================================================================
//  IVAsioPeer via IVAsioConnectionPeer
// ================================================================================

void VAsioProxyPeer::SendSilKitMsg(SerializedMessage buffer)
{
    ProxyMessage msg{};
    msg.source = _participantName;
    msg.destination = GetInfo().participantName;
    msg.payload = buffer.ReleaseStorage();

    Log::Trace(_logger, "VAsioProxyPeer ({}): SendSilKitMsg({})", _peerInfo.participantName, msg.payload.size());

    // keep track of aggregation kind
    auto bufferProxy = SerializedMessage{msg};
    bufferProxy.SetAggregationKind(buffer.GetAggregationKind());

    _peer->SendSilKitMsg(std::move(bufferProxy));
}

void VAsioProxyPeer::Subscribe(VAsioMsgSubscriber subscriber)
{
    Log::Debug(_logger, "VAsioProxyPeer: Subscribing to messages of type '{}' on link '{}' from participant '{}'",
               subscriber.msgTypeName, subscriber.networkName, _peerInfo.participantName);

    SendSilKitMsg(SerializedMessage{subscriber});
}

auto VAsioProxyPeer::GetInfo() const -> const VAsioPeerInfo&
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
    Log::Debug(_logger, "VAsioProxyPeer ({}): StartAsyncRead: Ignored", _peerInfo.participantName);
}

void VAsioProxyPeer::Shutdown()
{
    Log::Debug(_logger, "VAsioProxyPeer ({}): Shutdown: Ignored", _peerInfo.participantName);
}

void VAsioProxyPeer::EnableAggregation()
{
    Log::Debug(_logger, "VAsioProxyPeer ({}): EnableAggregation: Ignored", _peerInfo.participantName);
}

void VAsioProxyPeer::SetProtocolVersion(ProtocolVersion v)
{
    Log::Debug(_logger, "VAsioProxyPeer ({}): SetProtocolVersion: {}.{}", _peerInfo.participantName, v.major, v.minor);
    _protocolVersion = v;
}

auto VAsioProxyPeer::GetProtocolVersion() const -> ProtocolVersion
{
    Log::Trace(_logger, "VAsioProxyPeer ({}): GetProtocolVersion: {}.{}", _peerInfo.participantName,
               _protocolVersion.major, _protocolVersion.minor);
    return _protocolVersion;
}

void VAsioProxyPeer::SetSimulationName(const std::string& simulationName)
{
    _simulationName = simulationName;
}

auto VAsioProxyPeer::GetSimulationName() const -> const std::string&
{
    return _simulationName;
}

// ================================================================================
//  IServiceEndpoint via IVAsioConnectionPeer
// ================================================================================

void VAsioProxyPeer::SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto VAsioProxyPeer::GetServiceDescriptor() const -> const ServiceDescriptor&
{
    return _serviceDescriptor;
}

// ================================================================================
//  IVAsioPeerConnection
// ================================================================================

void VAsioProxyPeer::OnSocketData(IVAsioPeer* from, SerializedMessage&& buffer)
{
    _listener->OnSocketData(from, std::move(buffer));
}

void VAsioProxyPeer::OnPeerShutdown(IVAsioPeer* peer)
{
    _listener->OnPeerShutdown(peer);
}

// ================================================================================
//  VAsioProxyPeer
// ================================================================================

auto VAsioProxyPeer::GetPeer() const -> IVAsioPeer*
{
    return _peer;
}

} // namespace Core
} // namespace SilKit
