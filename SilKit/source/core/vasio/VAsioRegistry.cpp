// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/VAsioRegistry.hpp"

#include "services/logging/Logger.hpp"
#include "util/Uri.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "core/vasio/TransformAcceptorUris.hpp"
#include "core/vasio/VAsioConstants.hpp"

#include "services/metrics/MetricsReceiver.hpp"
#include "services/metrics/MetricsProcessor.hpp"
#include "services/metrics/MetricsManager.hpp"
#include "services/metrics/CreateMetricsSinksFromParticipantConfiguration.hpp"

#include "core/internal/traits/SilKitServiceConfigTraits.hpp"

#include "silkit/services/orchestration/string_utils.hpp"
#include "core/service/ServiceDatatypes.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>
#include <fmt/format.h>


namespace Log = SilKit::Services::Logging;


namespace SilKit {
namespace Core {

namespace {

constexpr uint64_t FIRST_GENERATION{1};

// TEMPORARY TRACE INSTRUMENTATION (enabled by the environment variable SILKIT_REGISTRY_TRACE=1)
// Prints everything the registry learns from ParticipantStatus, WorkflowConfiguration and ServiceDiscoveryEvent.
auto TraceEnabled() -> bool
{
    static const bool enabled{[] {
        const char* value = std::getenv("SILKIT_REGISTRY_TRACE");
        return value != nullptr && std::string{value} != "0";
    }()};
    return enabled;
}

void TraceEmit(const char* kind, const IServiceEndpoint* from, const std::string& details)
{
    static std::atomic<uint64_t> sequence{0};

    const auto now{std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count()};

    std::string senderInfo{"<null sender>"};
    if (from != nullptr)
    {
        const auto& sd{from->GetServiceDescriptor()};
        const auto generation{sd.GetGeneration()};
        senderInfo = fmt::format("sender='{}' sim='{}' gen={} service='{}' (id={})", sd.GetParticipantName(),
                                 sd.GetSimulationName(), generation ? std::to_string(*generation) : "none",
                                 sd.GetServiceName(), sd.GetServiceId());
    }

    // single write, so that lines of concurrent registries do not interleave
    std::cerr << fmt::format("[REGISTRY-TRACE #{} t={}us thread={}] {} | {} | {}\n", sequence++, now,
                             std::hash<std::thread::id>{}(std::this_thread::get_id()) % 100000, kind, senderInfo,
                             details);
}

auto TraceTimePoint(std::chrono::system_clock::time_point tp) -> int64_t
{
    return std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
}

} // namespace

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

bool VAsioRegistry::ShouldEnterGeneration(const std::string& participantName,
                                          const ConnectedParticipants& generation) const
{
    return generation.find(participantName) == generation.end();
}

auto VAsioRegistry::SelectGeneration(const std::string& simulationName, const std::string& participantName) const
    -> uint64_t
{
    const auto simulationIt{_simulationNames.find(simulationName)};
    if (simulationIt == _simulationNames.end())
    {
        return FIRST_GENERATION;
    }

    const auto& generations{simulationIt->second};

    // std::map iterates in ascending generation order
    for (const auto& generationPair : generations)
    {
        if (ShouldEnterGeneration(participantName, generationPair.second))
        {
            return generationPair.first;
        }
    }

    return generations.empty() ? FIRST_GENERATION : generations.rbegin()->first + 1;
}

bool VAsioRegistry::IsPeerRegistered(const IVAsioPeer* peer) const
{
    const auto generation{peer->GetServiceDescriptor().GetGeneration()};
    if (!generation.has_value())
    {
        return false;
    }

    const auto simulationIt{_simulationNames.find(peer->GetSimulationName())};
    if (simulationIt == _simulationNames.end())
    {
        return false;
    }

    const auto generationIt{simulationIt->second.find(*generation)};
    if (generationIt == simulationIt->second.end())
    {
        return false;
    }

    const auto participantIt{generationIt->second.find(peer->GetInfo().participantName)};
    return participantIt != generationIt->second.end() && participantIt->second.peer == peer;
}

void VAsioRegistry::OnParticipantAnnouncement(IVAsioPeer* peer, const ParticipantAnnouncement& announcement)
{
    const auto& peerInfo = announcement.peerInfo;

    // NB When we have a remote client we might need to patch its acceptor name (host or ip address).
    // In a distributed simulation the participants will listen on an IPADDRANY address
    // without explicitly specifying on which network interface they are listening.
    // When the IVAsioPeer connects to us we see its actual endpoint address and need
    // to substitute it here.

    const auto generation{SelectGeneration(announcement.simulationName, peerInfo.participantName)};

    if (TraceEnabled())
    {
        // snapshot of the registry state before this participant is registered
        std::string known;
        const auto simulationIt{_simulationNames.find(announcement.simulationName)};
        if (simulationIt != _simulationNames.end())
        {
            for (const auto& generationPair : simulationIt->second)
            {
                std::string names;
                for (const auto& participantPair : generationPair.second)
                {
                    names += (names.empty() ? "" : ", ") + participantPair.first;
                }
                known += fmt::format("{}gen {}: [{}]", known.empty() ? "" : "; ", generationPair.first, names);
            }
        }

        std::string uris;
        for (const auto& uri : peerInfo.acceptorUris)
        {
            uris += (uris.empty() ? "'" : ", '") + uri + "'";
        }

        TraceEmit("ParticipantAnnouncement", nullptr,
                  fmt::format("participant='{}' participantId={} simulationName='{}' protocol={}.{} "
                              "announcementHeader={}.{} capabilities='{}' acceptorUris=[{}] remoteAddress='{}' "
                              "localAddress='{}' peerSimulationName='{}' | selectedGeneration={} | "
                              "registryStateBefore={{{}}}",
                              peerInfo.participantName, peerInfo.participantId, announcement.simulationName,
                              peer->GetProtocolVersion().major, peer->GetProtocolVersion().minor,
                              announcement.messageHeader.versionHigh, announcement.messageHeader.versionLow,
                              peerInfo.capabilities, uris, peer->GetRemoteAddress(), peer->GetLocalAddress(),
                              peer->GetSimulationName(), generation, known));
    }

    GetLoggerInternal()->MakeMessage(Log::Level::Debug, TopicOf(*this))
        .SetMessage("Participant '{}' in simulation '{}' is placed into generation {}", peerInfo.participantName,
                    announcement.simulationName, generation)
        .Dispatch();

    SendKnownParticipants(peer, announcement.simulationName, generation);

    ConnectedParticipantInfo participantInfo;
    participantInfo.peer = peer;
    participantInfo.peerInfo = peerInfo;
    _simulationNames[announcement.simulationName][generation][peerInfo.participantName] = participantInfo;

    // The generation is stamped on the service descriptor of the peer, like the simulation name, so that receivers of
    // messages and the connection can read it from there. Names are not unique across generations, so peers are
    // identified by pointer.
    auto peerDescriptor{peer->GetServiceDescriptor()};
    peerDescriptor.SetGeneration(generation);
    peer->SetServiceDescriptor(peerDescriptor);

    if (_registryEventListener != nullptr)
    {
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

void VAsioRegistry::SendKnownParticipants(IVAsioPeer* peer, const std::string& simulationName, uint64_t generation)
{
    GetLoggerInternal()->MakeMessage(Log::Level::Info, TopicOf(*this))
        .SetMessage("Sending known participant message to {}, protocol version {}.{}",
                            peer->GetInfo().participantName, peer->GetProtocolVersion().major,
                            peer->GetProtocolVersion().minor)
        .Dispatch();

    KnownParticipants knownParticipantsMsg;
    knownParticipantsMsg.messageHeader = MakeRegistryMsgHeader(peer->GetProtocolVersion());

    const auto& simulationParticipants{_simulationNames[simulationName][generation]};

    for (const auto& pPair : simulationParticipants)
    {
        const auto& connectedParticipant{pPair.second};

        // don't advertise the peer to itself
        if (connectedParticipant.peer == peer)
            continue;

        auto peerInfo = connectedParticipant.peerInfo;
        peerInfo.acceptorUris = TransformAcceptorUris(GetLoggerInternal(), connectedParticipant.peer, peer);

        knownParticipantsMsg.peerInfos.push_back(peerInfo);
    }

    peer->SendSilKitMsg(SerializedMessage{peer->GetProtocolVersion(), knownParticipantsMsg});
}

void VAsioRegistry::OnPeerShutdown(IVAsioPeer* peer)
{
    const auto& simulationName{peer->GetSimulationName()};
    const auto& participantName{peer->GetInfo().participantName};

    const auto generation{peer->GetServiceDescriptor().GetGeneration()};

    if (TraceEnabled())
    {
        TraceEmit("PeerShutdown", peer,
                  fmt::format("participant='{}' simulationName='{}' generation={} registered={}", participantName,
                              simulationName, generation ? std::to_string(*generation) : "none",
                              IsPeerRegistered(peer)));
    }

    // The generation is stamped on the peer and outlives its registration. Only a peer that is still registered
    // under its name in its generation is removed, a different peer with the same name must not be affected.
    if (!IsPeerRegistered(peer))
    {
        GetLoggerInternal()->MakeMessage(Log::Level::Error, TopicOf(*this))
            .SetMessage("Peer '{}' has shut down, which had no participant information", participantName)
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

    auto& generations{_simulationNames[simulationName]};
    auto& participants{generations[*generation]};
    participants.erase(participantName);

    if (participants.empty())
    {
        generations.erase(*generation);
    }

    if (generations.empty())
    {
        _simulationNames.erase(simulationName);
    }

    if (_simulationNames.empty())
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
    if (TraceEnabled())
    {
        TraceEmit("ParticipantStatus", from,
                  fmt::format("status.participantName='{}' state={} enterReason='{}' enterTime={}us refreshTime={}us "
                              "senderMatchesStatusParticipant={}",
                              msg.participantName, to_string(msg.state), msg.enterReason,
                              TraceTimePoint(msg.enterTime), TraceTimePoint(msg.refreshTime),
                              from != nullptr && from->GetServiceDescriptor().GetParticipantName()
                                  == msg.participantName));
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
    if (TraceEnabled())
    {
        std::string names;
        for (const auto& name : msg.requiredParticipantNames)
        {
            names += (names.empty() ? "'" : ", '") + name + "'";
        }
        TraceEmit("WorkflowConfiguration", from,
                  fmt::format("requiredParticipantNames=[{}] (count={})", names, msg.requiredParticipantNames.size()));
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
    if (TraceEnabled())
    {
        const auto& sd{msg.serviceDescriptor};
        const auto& supplemental{sd.GetSupplementalDataRef()};

        std::string supplementalText;
        for (const auto& kv : supplemental)
        {
            supplementalText += fmt::format("{}{}='{}'", supplementalText.empty() ? "" : ", ", kv.first, kv.second);
        }

        const auto valueOf = [&supplemental](const std::string& key) {
            const auto it{supplemental.find(key)};
            return it == supplemental.end() ? std::string{"<unset>"} : it->second;
        };

        std::ostringstream type;
        type << msg.type;

        TraceEmit("ServiceDiscoveryEvent", from,
                  fmt::format("type={} descriptor.participant='{}' descriptor.sim='{}' descriptor.gen={} "
                              "service='{}' (id={}) network='{}' controllerType={} lifecycleIsCoordinated={} "
                              "timeSyncActive={} supplemental={{{}}}",
                              type.str(), sd.GetParticipantName(), sd.GetSimulationName(),
                              sd.GetGeneration() ? std::to_string(*sd.GetGeneration()) : "none", sd.GetServiceName(),
                              sd.GetServiceId(), sd.GetNetworkName(),
                              valueOf(SilKit::Core::Discovery::controllerType),
                              valueOf(SilKit::Core::Discovery::lifecycleIsCoordinated),
                              valueOf(SilKit::Core::Discovery::timeSyncActive), supplementalText));
    }

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
