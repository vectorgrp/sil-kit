// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioRegistry.hpp"

#include "services/logging/Logger.hpp"
#include "util/Uri.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "core/vasio/TransformAcceptorUris.hpp"
#include "core/vasio/VAsioProtocolVersion.hpp"
#include "core/vasio/VAsioConstants.hpp"

#include "services/metrics/MetricsReceiver.hpp"
#include "services/metrics/MetricsProcessor.hpp"
#include "services/metrics/MetricsManager.hpp"
#include "services/metrics/CreateMetricsSinksFromParticipantConfiguration.hpp"

#include "core/internal/traits/SilKitServiceConfigTraits.hpp"

#include "core/service/ServiceDatatypes.hpp"

#include <fmt/format.h>


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
        _registryEventListener->OnLoggerCreated(_logger.get());
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
    catch (const SilKit::ConfigurationError& e)
    {
        // An unusable listen address is a user error. Falling back to domain sockets would leave a registry that no
        // participant can reach over TCP.
        GetLoggerInternal()->MakeMessage(Log::Level::Error, TopicOf(*this))
            .SetMessage("SIL Kit Registry cannot listen on {}. Reason: {}", uri.EncodedString(), e.what())
            .Dispatch();
        throw;
    }
    catch (const std::exception& e)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Error, TopicOf(*this))
            .SetMessage("SIL Kit Registry failed to create listening socket {}:{} (uri: {}). Reason: {}",
                                 uri.Host(), uri.Port(), uri.EncodedString(), e.what())
            .Dispatch();
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
            GetLoggerInternal()->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage("SIL Kit Registry failed to create local listening socket: {}", e.what())
                .Dispatch();
        }
    }

    if (hasTcpSocket && hasDomainSocket)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Debug, TopicOf(*this))
            .SetMessage("SIL Kit Registry: Listening on both, TCP and Domain sockets")
            .Dispatch();
    }
    else if (hasTcpSocket && !hasDomainSocket)
    {
        if (enableDomainSockets)
        {
            GetLoggerInternal()->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage("This registry instance will only accept connections on TCP sockets. This might be caused by a second "
                "registry running on this host, or using an operating system that does not support Domain sockets.")
                .Dispatch();
        }
        else
        {
            GetLoggerInternal()->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage("This registry instance will only accept connections on TCP sockets. Domain sockets were "
                          "explicitly disabled through the participant configuration.")
                .Dispatch();
        }
    }
    else if (!hasTcpSocket && hasDomainSocket)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Warn, TopicOf(*this))
            .SetMessage("This registry instance will only accept connections on local domain sockets. This might be "
                      "caused by a second registry running on this host, or another process was already bound to the "
                      "same port as this registry was attempting to use.")
            .Dispatch();
    }
    else
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Error, TopicOf(*this))
            .SetMessage("SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets")
            .Dispatch();
        throw SilKit::StateError{"SIL Kit Registry: Unable to listen on neither TCP, nor Domain sockets"};
    }

    if (_registryEventListener != nullptr)
    {
        if (!_vasioConfig->experimental.metrics.collectFromRemote.value_or(false))
        {
            /* Without this the registry never receives a MetricsUpdate, so the dashboard shows no
             * metrics and - because attributes travel as metrics - no participant attributes
             * either. Nothing else reports the combination, and the result is indistinguishable
             * from a broken dashboard connection. */
            GetLoggerInternal()
                ->MakeMessage(Log::Level::Warn, Log::Topic::Dashboard)
                .SetMessage("SIL Kit Registry: the dashboard is enabled but Experimental.Metrics.CollectFromRemote "
                            "is disabled, so no metrics and no participant attributes will reach the dashboard")
                .Dispatch();
        }

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
    return _logger ? _logger->AsILogger() : nullptr;
}

auto VAsioRegistry::GetLoggerInternal() -> Services::Logging::ILoggerInternal*
{
    return _logger.get();
}

auto VAsioRegistry::GetOrCreatePlacement(const std::string& simulationName) -> SessionPlacement*
{
    if (simulationName.compare(0, 8, "session/") != 0)
    {
        return nullptr;
    }

    auto& placement{_placements[simulationName]};
    if (placement == nullptr)
    {
        placement = std::make_unique<SessionPlacement>(simulationName, GetLoggerInternal());
    }
    return placement.get();
}

auto VAsioRegistry::FindPlacement(const std::string& simulationName) const -> SessionPlacement*
{
    const auto placementIt{_placements.find(simulationName)};
    return placementIt == _placements.end() ? nullptr : placementIt->second.get();
}

auto VAsioRegistry::FindPlacementOfSender(const IServiceEndpoint* from) const -> SessionPlacement*
{
    if (from == nullptr)
    {
        return nullptr;
    }

    return FindPlacement(from->GetServiceDescriptor().GetSimulationName());
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement)
{
    const auto& peerInfo = announcement.peerInfo;

    const auto registered{_connection.ParticipantsByGeneration(announcement.simulationName)};

    if (auto* placement = GetOrCreatePlacement(announcement.simulationName))
    {
        placement->Admit(peer, registered);
    }
    else
    {
        const auto generationIt{registered.find(0)};
        if (generationIt != registered.end() && generationIt->second.count(peerInfo.participantName) != 0)
        {
            const auto message = fmt::format("A participant with the same name '{}' already exists in the simulation {}",
                                             peerInfo.participantName, announcement.simulationName);
            GetLoggerInternal()->MakeMessage(Log::Level::Warn, TopicOf(*this))
                .SetMessage(message)
                .Dispatch();
            throw SilKitError{message};
        }
    }

    SendKnownParticipants(peer);

    if (_registryEventListener != nullptr)
    {
        const auto generation{peer->GetServiceDescriptor().GetGeneration()};
        if (generation.has_value() && registered.count(*generation) == 0)
        {
            _registryEventListener->OnGenerationCreated(announcement.simulationName, *generation);
        }
        _registryEventListener->OnParticipantConnected(announcement.simulationName, peerInfo.participantName);
    }

    if (AllParticipantsAreConnected())
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
            .SetMessage("All participants are online")
            .Dispatch();
        if (_onAllParticipantsConnected)
            _onAllParticipantsConnected();
    }
}

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer)
{
    GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
        .SetMessage("Sending known participant message to {}, protocol version {}.{}",
                            peer->GetInfo().participantName, peer->GetProtocolVersion().major,
                            peer->GetProtocolVersion().minor)
        .Dispatch();

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());

    for (auto* knownPeer : _connection.PeersInGenerationOf(peer))
    {
        auto peerInfo = knownPeer->GetInfo();
        // Replace wildcard acceptor hosts with the address observed by the registry.
        peerInfo.acceptorUris = TransformAcceptorUris(GetLoggerInternal(), knownPeer, peer);

        knownParticipantsMsg.peerInfos.push_back(std::move(peerInfo));
    }

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    const auto& simulationName{peer->GetSimulationName()};
    const auto& participantName{peer->GetInfo().participantName};

    const auto registration{_connection.GetPeerRegistration(peer)};

    if (!registration.registered)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Debug, TopicOf(*this))
            .SetMessage(registration.nameTakenByOther
                            ? "Duplicate peer '{}' has shut down, which had no participant information"
                            : "Peer '{}' has shut down, which had no participant information",
                        participantName)
            .Dispatch();
        return;
    }

    GetLoggerInternal()->MakeMessage(Log::Level::Debug, TopicOf(*this))
        .SetMessage("Peer '{}' has shut down", participantName)
        .Dispatch();

    if (_registryEventListener != nullptr)
    {
        _registryEventListener->OnParticipantDisconnected(simulationName, participantName);
    }

    if (auto* placement = FindPlacement(simulationName))
    {
        placement->OnPeerLeaving(peer, registration.othersInGeneration == 0);
    }

    if (registration.othersInSimulation == 0)
    {
        _placements.erase(simulationName);
    }

    if (registration.othersTotal == 0)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
            .SetMessage("All participants are shut down")
            .Dispatch();
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
    if (auto* placement = FindPlacementOfSender(from))
    {
        placement->OnStatus(from, msg,
                            _connection.ParticipantsByGeneration(from->GetServiceDescriptor().GetSimulationName()));
    }

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
    if (auto* placement = FindPlacementOfSender(from))
    {
        placement->OnWorkflow(from, msg,
                              _connection.ParticipantsByGeneration(from->GetServiceDescriptor().GetSimulationName()));
    }

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
    GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
        .SetMessage("Participant {} updates {} metrics", participantName, metricsUpdate.metrics.size())
        .Dispatch();
        
    for (const auto& data : metricsUpdate.metrics)
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
            .SetMessage("Metric Update: {} {} {} {} ({})", data.name, data.kind, data.value, data.timestamp,
                  participantName)
            .Dispatch();
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
