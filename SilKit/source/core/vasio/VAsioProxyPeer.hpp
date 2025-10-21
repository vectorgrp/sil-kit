// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IVAsioPeer.hpp"


namespace SilKit {
namespace Services {
namespace Logging {
class ILogger;
} // namespace Logging
} // namespace Services
} // namespace SilKit


namespace SilKit {
namespace Core {


class VAsioProxyPeer
    : public IVAsioPeer
    , public IVAsioPeerListener
{
public:
    VAsioProxyPeer(IVAsioPeerListener* listener, std::string participantName, VAsioPeerInfo peerInfo, IVAsioPeer* peer,
                   SilKit::Services::Logging::ILogger* logger);

public: // IVAsioPeer
    void SendSilKitMsg(SerializedMessage buffer) override;
    void Subscribe(VAsioMsgSubscriber subscriber) override;
    auto GetInfo() const -> const VAsioPeerInfo& override;
    void SetInfo(VAsioPeerInfo info) override;
    auto GetRemoteAddress() const -> std::string override;
    auto GetLocalAddress() const -> std::string override;
    void StartAsyncRead() override;
    void Shutdown() override;
    void EnableAggregation() override;
    void SetProtocolVersion(ProtocolVersion v) override;
    auto GetProtocolVersion() const -> ProtocolVersion override;
    void SetSimulationName(const std::string& simulationName) override;
    auto GetSimulationName() const -> const std::string& override;

    void InitializeMetrics(VSilKit::IMetricsManager*) override {}

public: // IVAsioPeer (IServiceEndpoint)
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override;

public: // IVAsioPeerListener
    void OnSocketData(IVAsioPeer* from, SerializedMessage&& buffer) override;
    void OnPeerShutdown(IVAsioPeer* peer) override;

public:
    auto GetPeer() const -> IVAsioPeer*;

private:
    IVAsioPeerListener* _listener;
    std::string _participantName;
    std::string _simulationName;
    IVAsioPeer* _peer;
    VAsioPeerInfo _peerInfo;
    ServiceDescriptor _serviceDescriptor;
    SilKit::Services::Logging::ILogger* _logger;
    ProtocolVersion _protocolVersion;
};


} // namespace Core
} // namespace SilKit
