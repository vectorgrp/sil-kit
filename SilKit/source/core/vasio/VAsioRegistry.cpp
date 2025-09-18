// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "VAsioRegistry.hpp"

#include "Logger.hpp"
#include "Uri.hpp"
#include "LoggerMessage.hpp"
#include "TransformAcceptorUris.hpp"
#include "VAsioConstants.hpp"

#include "MetricsReceiver.hpp"
#include "MetricsProcessor.hpp"
#include "MetricsManager.hpp"
#include "CreateMetricsSinksFromParticipantConfiguration.hpp"

#include "traits/SilKitServiceConfigTraits.hpp"


namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {


VAsioRegistry::VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg, ProtocolVersion version)
    : VAsioRegistry(std::move(cfg), nullptr, version)
{
}

VAsioRegistry::VAsioRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> cfg,
                             IRegistryEventListener* registryEventListener, ProtocolVersion version)
    : _registryEventListener{registryEventListener}
    , _vasioConfig{std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(cfg)}
    , _metricsProcessor{std::make_unique<VSilKit::MetricsProcessor>(REGISTRY_PARTICIPANT_NAME)}
    , _metricsManager{std::make_unique<VSilKit::MetricsManager>(REGISTRY_PARTICIPANT_NAME, *_metricsProcessor)}
    , _connection{nullptr,
                  _metricsManager.get(),
                  *_vasioConfig,
                  REGISTRY_PARTICIPANT_NAME,
                  REGISTRY_PARTICIPANT_ID,
                  &_timeProvider,
                  version}
{
    _logger = std::make_unique<Services::Logging::Logger>(REGISTRY_PARTICIPANT_NAME, _vasioConfig->logging);

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnLoggerCreated(dynamic_cast<SilKit::Services::Logging::ILogger*>(_logger.get()));
    }

    dynamic_cast<VSilKit::MetricsProcessor&>(*_metricsProcessor).SetLogger(*_logger);
    dynamic_cast<VSilKit::MetricsManager&>(*_metricsManager).SetLogger(*_logger);
    _connection.SetLoggerInternal(_logger.get());

    _connection.RegisterMessageReceiver([this](IVAsioPeer* from, const ParticipantAnnouncement& announcement) {
        this->OnParticipantAnnouncement(from, announcement);
    });

    _connection.RegisterPeerShutdownCallback([this](IVAsioPeer* peer) { OnPeerShutdown(peer); });

    _serviceDescriptor.SetParticipantNameAndComputeId(REGISTRY_PARTICIPANT_NAME);
    _serviceDescriptor.SetParticipantId(REGISTRY_PARTICIPANT_ID);
    _serviceDescriptor.SetNetworkName("default");
    _serviceDescriptor.SetServiceId(_localEndpointId++);

    SetupMetrics();
}

auto VAsioRegistry::StartListening(const std::string& listenUri) -> std::string
{
    auto uri = Uri::Parse(listenUri);
    const auto enableDomainSockets = _vasioConfig->middleware.enableDomainSockets;

    bool hasTcpSocket = false;
    bool hasDomainSocket = false;

    try
    {
        // Resolve the configured hostname and accept on the given port:
        auto tcpHostPort = _connection.AcceptTcpConnectionsOn(uri.Host(), uri.Port());
        // Update the URI to the actually used address and port:
        uri = Uri{uri.Host(), tcpHostPort.second};

        hasTcpSocket = true;
    }
    catch (const std::exception& e)
    {
        Services::Logging::Error(GetLogger(),
                                 "SIL Kit Registry failed to create listening socket {}:{} (uri: {}). Reason: {}",
                                 uri.Host(), uri.Port(), uri.EncodedString(), e.what());
    }

    if (enableDomainSockets)
    {
        try
        {
            // If we have opened a TCP socket, use the URI we're going to return to the caller. Otherwise, use the URI
            // passed as the argument.
            std::string uniqueId{listenUri};
            if (hasTcpSocket)
            {
                uniqueId = uri.EncodedString();
            }

            _connection.AcceptLocalConnections(uniqueId);

            hasDomainSocket = true;
        }
        catch (const std::exception& e)
        {
            Services::Logging::Warn(GetLogger(), "SIL Kit Registry failed to create local listening socket: {}",
                                    e.what());
        }
    }

    if (hasTcpSocket && hasDomainSocket)
    {
        Services::Logging::Debug(GetLogger(), "SIL Kit Registry: Listening on both, TCP and Domain sockets");
    }
    else if (hasTcpSocket && !hasDomainSocket)
    {
        if (enableDomainSockets)
        {
            // There exist old versions of Windows that do not support domain sockets. Here only TCP/IP will be available.
            _logger->Warn(
                "This registry instance will only accept connections on TCP sockets. This might be caused by a second "
                "registry running on this host, or using an operating system that does not support Domain sockets.");
        }
        else
        {
            _logger->Warn("This registry instance will only accept connections on TCP sockets. Domain sockets were "
                          "explicitly disabled through the participant configuration.");
        }
    }
    else if (!hasTcpSocket && hasDomainSocket)
    {
        // For scenarios where multiple instances run on the same host, binding on TCP/IP will result in an error.
        // However, if we can accept local ipc connections we warn and continue.
        _logger->Warn("This registry instance will only accept connections on local domain sockets. This might be "
                      "caused by a second registry running on this host, or another process was already bound to the "
                      "same port as this registry was attempting to use.");
    }
    else
    {
        Services::Logging::Error(GetLogger(), "SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets");
        throw SilKit::StateError{"SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets"};
    }

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnRegistryUri(uri.EncodedString());
    }

    _connection.StartIoWorker();

    _connection.RegisterSilKitService(this);
    if (_vasioConfig->experimental.metrics.collectFromRemote.value_or(false))
    {
        _connection.RegisterSilKitService(_metricsReceiver.get());
    }

    return uri.EncodedString();
}

void VAsioRegistry::SetAllConnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsConnected = std::move(handler);
}
void VAsioRegistry::SetAllDisconnectedHandler(std::function<void()> handler)
{
    _onAllParticipantsDisconnected = std::move(handler);
}
auto VAsioRegistry::GetLogger() -> Services::Logging::ILogger*
{
    return _logger.get();
}

auto VAsioRegistry::FindConnectedParticipant(const std::string& participantName,
                                             const std::string& simulationName) const -> const ConnectedParticipantInfo*
{
    const auto simulationIt{_connectedParticipants.find(simulationName)};
    if (simulationIt == _connectedParticipants.end())
    {
        return nullptr;
    }

    const auto& simulationParticipants{simulationIt->second};

    const auto participantIt{simulationParticipants.find(participantName)};
    if (participantIt == simulationParticipants.end())
    {
        return nullptr;
    }

    return std::addressof(participantIt->second);
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement)
{
    const auto& peerInfo = announcement.peerInfo;

    // NB When we have a remote client we might need to patch its acceptor name (host or ip address).
    // In a distributed simulation the participants will listen on an IPADDRANY address
    // without explicitly specifying on which network interface they are listening.
    // When the IVAsioPeer connects to us we see its actual endpoint address and need
    // to substitute it here.

    // Do not allow multiple participants with identical names
    if (FindConnectedParticipant(peerInfo.participantName, announcement.simulationName) != nullptr)
    {
        const auto message = fmt::format("A participant with the same name '{}' already exists in the simulation {}",
                                         peerInfo.participantName, announcement.simulationName);
        GetLogger()->Warn(message);
        throw SilKitError{message};
    }

    SendKnownParticipants(peer, announcement.simulationName);

    ConnectedParticipantInfo participantInfo;
    participantInfo.peer = peer;
    participantInfo.peerInfo = peerInfo;
    _connectedParticipants[announcement.simulationName][peerInfo.participantName] = participantInfo;

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnParticipantConnected(announcement.simulationName, peerInfo.participantName);
    }

    if (AllParticipantsAreConnected())
    {
        _logger->Info("All participants are online");
        if (_onAllParticipantsConnected)
            _onAllParticipantsConnected();
    }
}

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName)
{
    Services::Logging::Info(GetLogger(), "Sending known participant message to {}, protocol version {}.{}",
                            peer->GetInfo().participantName, peer->GetProtocolVersion().major,
                            peer->GetProtocolVersion().minor);

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());

    const auto& simulationParticipants{_connectedParticipants[simulationName]};

    for (const auto& pPair : simulationParticipants)
    {
        const auto& connectedParticipant{pPair.second};

        // don't advertise the peer to itself
        if (connectedParticipant.peer == peer)
            continue;

        auto peerInfo = connectedParticipant.peerInfo;
        peerInfo.acceptorUris = TransformAcceptorUris(GetLogger(), connectedParticipant.peer, peer);

        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    namespace Log = SilKit::Services::Logging;

    const auto& simulationName{peer->GetSimulationName()};
    const auto& participantName{peer->GetInfo().participantName};

    const auto connectedParticipant = FindConnectedParticipant(participantName, simulationName);

    if (connectedParticipant == nullptr)
    {
        Log::Debug(_logger.get(), "Peer '{}' has shut down, which had no participant information", participantName);
        return;
    }

    if (connectedParticipant->peer != peer)
    {
        Log::Debug(_logger.get(), "Duplicate peer '{}' has shut down, which had no participant information",
                   participantName);
        return;
    }

    Log::Debug(_logger.get(), "Peer '{}' has shut down", participantName);

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnParticipantDisconnected(simulationName, participantName);
    }

    _connectedParticipants[simulationName].erase(participantName);

    if (_connectedParticipants[simulationName].empty())
    {
        _connectedParticipants.erase(simulationName);
    }

    if (_connectedParticipants.empty())
    {
        _logger->Info("All participants are shut down");
        if (_onAllParticipantsDisconnected)
            _onAllParticipantsDisconnected();
    }
}

bool VAsioRegistry::AllParticipantsAreConnected() const
{
    return false;
}

void VAsioRegistry::SetupMetrics()
{
    auto& processor = dynamic_cast<VSilKit::MetricsProcessor&>(*_metricsProcessor);
    {
        // the registry has no MetricsSender
        auto sinks = VSilKit::CreateMetricsSinksFromParticipantConfiguration(
            _logger.get(), nullptr, REGISTRY_PARTICIPANT_NAME, _vasioConfig->experimental.metrics.sinks);
        processor.SetSinks(std::move(sinks));
    }

    if (_vasioConfig->experimental.metrics.collectFromRemote.value_or(false))
    {
        auto metricsReceiver = std::make_unique<VSilKit::MetricsReceiver>(nullptr, *this);

        SilKit::Core::SupplementalData supplementalData;
        supplementalData[SilKit::Core::Discovery::controllerType] =
            SilKit::Core::Discovery::controllerTypeMetricsReceiver;

        auto sd = metricsReceiver->GetServiceDescriptor();
        sd.SetParticipantNameAndComputeId(_serviceDescriptor.GetParticipantName());
        sd.SetParticipantId(_serviceDescriptor.GetParticipantId());
        sd.SetNetworkName("default");
        sd.SetNetworkType(SilKit::Config::NetworkType::Undefined);
        sd.SetServiceName("MetricsReceiver");
        sd.SetServiceId(_localEndpointId++);
        sd.SetServiceType(SilKitServiceTraitServiceType<VSilKit::MetricsReceiver>::GetServiceType());
        sd.SetSupplementalData(std::move(supplementalData));

        metricsReceiver->SetServiceDescriptor(sd);

        _metricsReceiver = std::move(metricsReceiver);
    }
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Services::Orchestration::ParticipantStatus& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnParticipantStatusUpdate(serviceDescriptor.GetSimulationName(),
                                                      serviceDescriptor.GetParticipantName(), msg);
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Services::Orchestration::WorkflowConfiguration& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnRequiredParticipantsUpdate(
        serviceDescriptor.GetSimulationName(), serviceDescriptor.GetParticipantName(), msg.requiredParticipantNames);
}

void VAsioRegistry::ReceiveMsg(const SilKit::Core::IServiceEndpoint* from,
                               const SilKit::Core::Discovery::ServiceDiscoveryEvent& msg)
{
    if (_registryEventListener == nullptr)
    {
        return;
    }

    const auto& serviceDescriptor{from->GetServiceDescriptor()};

    _registryEventListener->OnServiceDiscoveryEvent(serviceDescriptor.GetSimulationName(),
                                                    serviceDescriptor.GetParticipantName(), msg);
}

void VAsioRegistry::SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto VAsioRegistry::GetServiceDescriptor() const -> const ServiceDescriptor&
{
    return _serviceDescriptor;
}


void VAsioRegistry::OnMetricsUpdate(const std::string& simulationName, const std::string& participantName,
                                    const VSilKit::MetricsUpdate& metricsUpdate)
{
    Log::Info(GetLogger(), "Participant {} updates {} metrics", participantName, metricsUpdate.metrics.size());
    for (const auto& data : metricsUpdate.metrics)
    {
        Log::Info(GetLogger(), "Metric Update: {} {} {} {} ({})", data.name, data.kind, data.value, data.timestamp,
                  participantName);
    }

    dynamic_cast<VSilKit::MetricsProcessor&>(*_metricsProcessor)
        .OnMetricsUpdate(simulationName, participantName, metricsUpdate);

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnMetricsUpdate(simulationName, participantName, metricsUpdate);
    }
}


} // namespace Core
} // namespace SilKit
