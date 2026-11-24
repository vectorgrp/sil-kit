// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "management/RegistryManagementParticipant.hpp"

#include "silkit/services/pubsub/IDataPublisher.hpp"
#include "silkit/services/rpc/IRpcServer.hpp"

#include "CreateParticipantImpl.hpp"
#include "SilKitVersionImpl.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "util/Uri.hpp"

namespace Log = SilKit::Services::Logging;
namespace Mgmt = SilKit::Experimental::Management;

using SilKit::Services::PubSub::PubSubSpec;
using SilKit::Services::Rpc::IRpcServer;
using SilKit::Services::Rpc::RpcCallEvent;
using SilKit::Services::Rpc::RpcSpec;

namespace VSilKit {

namespace {

//! The registry may listen on a wildcard address, which is not a valid address to connect to.
auto MakeConnectableManagementUri(const std::string& registryUri) -> std::string
{
    const SilKit::Core::Uri uri{registryUri};
    auto host = uri.Host();
    if (host == "0.0.0.0")
    {
        host = "127.0.0.1";
    }
    else if (host == "[::]")
    {
        host = "[::1]";
    }
    return SilKit::Core::Uri::MakeSilKit(host, uri.Port(), Mgmt::SimulationName()).EncodedString();
}

} // namespace

auto CreateRegistryManagementParticipant(SilKit::Services::Logging::ILoggerInternal* logger,
                                         const SilKit::Config::ParticipantConfiguration& registryConfiguration)
    -> std::unique_ptr<IRegistryManagementParticipant>
{
    return std::make_unique<RegistryManagementParticipant>(logger,
                                                           registryConfiguration.middleware.enableDomainSockets);
}

RegistryManagementParticipant::RegistryManagementParticipant(SilKit::Services::Logging::ILoggerInternal* logger,
                                                             bool enableDomainSockets)
    : _logger{logger}
    , _enableDomainSockets{enableDomainSockets}
{
}

RegistryManagementParticipant::~RegistryManagementParticipant()
{
    Stop();
}

void RegistryManagementParticipant::Start(const std::string& registryUri)
{
    try
    {
        const auto managementUri = MakeConnectableManagementUri(registryUri);

        auto config = std::make_shared<SilKit::Config::ParticipantConfiguration>();
        config->middleware.enableDomainSockets = _enableDomainSockets;

        auto participant = SilKit::CreateParticipantImpl(config, Mgmt::RegistryParticipantName(), managementUri);

        const std::string mediaType{Mgmt::MediaType()};

        auto* infoPublisher =
            participant->CreateDataPublisher("RegistryInfo", PubSubSpec{Mgmt::Registry::InfoTopic(), mediaType}, 1);
        auto* simulationsPublisher = participant->CreateDataPublisher(
            "RegistrySimulations", PubSubSpec{Mgmt::Registry::SimulationsTopic(), mediaType}, 1);

        participant->CreateRpcServer("RegistryGetInfo", RpcSpec{Mgmt::Registry::GetInfoFunction(), mediaType},
                                     [this](IRpcServer* server, const RpcCallEvent& event) {
            server->SubmitResult(event.callHandle, Mgmt::Serialize(GetRegistryInfo()));
        });
        participant->CreateRpcServer("RegistryGetSimulations",
                                     RpcSpec{Mgmt::Registry::GetSimulationsFunction(), mediaType},
                                     [this](IRpcServer* server, const RpcCallEvent& event) {
            server->SubmitResult(event.callHandle, Mgmt::Serialize(GetSimulations()));
        });
        participant->CreateRpcServer("RegistryGetParticipantConfiguration",
                                     RpcSpec{Mgmt::Registry::GetParticipantConfigurationFunction(), mediaType},
                                     [this](IRpcServer* server, const RpcCallEvent& event) {
            Mgmt::ParticipantConfigurationReply reply;
            try
            {
                reply = GetParticipantConfiguration(
                    Mgmt::Deserialize<Mgmt::ParticipantConfigurationRequest>(event.argumentData));
            }
            catch (const std::exception& e)
            {
                _logger->MakeMessage(Log::Level::Warn, Log::Topic::None)
                    .SetMessage("Registry management participant: invalid participant configuration request: {}",
                                e.what())
                    .Dispatch();
            }
            server->SubmitResult(event.callHandle, Mgmt::Serialize(reply));
        });

        infoPublisher->Publish(Mgmt::Serialize(GetRegistryInfo()));

        std::lock_guard<decltype(_mutex)> lock{_mutex};
        _participant = std::move(participant);
        _simulationsPublisher = simulationsPublisher;
        PublishSimulations();

        _logger->MakeMessage(Log::Level::Info, Log::Topic::None)
            .SetMessage("Registry management participant joined {}", managementUri)
            .Dispatch();
    }
    catch (const std::exception& e)
    {
        _logger->MakeMessage(Log::Level::Error, Log::Topic::None)
            .SetMessage("Registry management participant failed to join the management simulation: {}", e.what())
            .Dispatch();
    }
}

void RegistryManagementParticipant::Stop()
{
    std::unique_ptr<SilKit::IParticipant> participant;
    {
        std::lock_guard<decltype(_mutex)> lock{_mutex};
        participant = std::move(_participant);
        _simulationsPublisher = nullptr;
    }
    // destroyed outside the lock: the RPC handlers of the participant take the lock
    participant.reset();
}

void RegistryManagementParticipant::OnParticipantConfiguration(const std::string& simulationName,
                                                               const std::string& participantName,
                                                               const std::string& configurationJson)
{
    if (IsWellKnownSimulation(simulationName))
    {
        return;
    }

    std::lock_guard<decltype(_mutex)> lock{_mutex};
    _simulations[simulationName].participants[participantName].configurationJson = configurationJson;
}

void RegistryManagementParticipant::OnParticipantConnected(const std::string& simulationName,
                                                           const std::string& participantName)
{
    if (IsWellKnownSimulation(simulationName))
    {
        return;
    }

    std::lock_guard<decltype(_mutex)> lock{_mutex};
    _simulations[simulationName].participants[participantName];
    PublishSimulations();
}

void RegistryManagementParticipant::OnParticipantDisconnected(const std::string& simulationName,
                                                              const std::string& participantName)
{
    if (IsWellKnownSimulation(simulationName))
    {
        return;
    }

    std::lock_guard<decltype(_mutex)> lock{_mutex};
    const auto simulationIt = _simulations.find(simulationName);
    if (simulationIt == _simulations.end())
    {
        return;
    }

    simulationIt->second.participants.erase(participantName);
    if (simulationIt->second.participants.empty())
    {
        _simulations.erase(simulationIt);
    }
    PublishSimulations();
}

void RegistryManagementParticipant::OnRequiredParticipantsUpdate(
    const std::string& simulationName, const std::string& /*participantName*/,
    SilKit::Util::Span<const std::string> requiredParticipantNames)
{
    if (IsWellKnownSimulation(simulationName))
    {
        return;
    }

    std::lock_guard<decltype(_mutex)> lock{_mutex};
    const auto simulationIt = _simulations.find(simulationName);
    if (simulationIt == _simulations.end())
    {
        return;
    }

    simulationIt->second.requiredParticipantNames = SilKit::Util::ToStdVector(requiredParticipantNames);
    PublishSimulations();
}

auto RegistryManagementParticipant::GetRegistryInfo() const -> Mgmt::RegistryInfo
{
    Mgmt::RegistryInfo info;
    info.apiVersionMajor = Mgmt::ApiVersionMajor();
    info.apiVersionMinor = Mgmt::ApiVersionMinor();
    info.silKitVersion = SilKit::Version::StringImpl();
    info.features = {Mgmt::Registry::FeatureSimulations(), Mgmt::Registry::FeatureParticipantConfiguration()};
    return info;
}

auto RegistryManagementParticipant::GetSimulations() const -> Mgmt::Simulations
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};
    return MakeSimulations();
}

auto RegistryManagementParticipant::GetParticipantConfiguration(
    const Mgmt::ParticipantConfigurationRequest& request) const -> Mgmt::ParticipantConfigurationReply
{
    Mgmt::ParticipantConfigurationReply reply;

    std::lock_guard<decltype(_mutex)> lock{_mutex};
    const auto simulationIt = _simulations.find(request.simulationName);
    if (simulationIt == _simulations.end())
    {
        return reply;
    }

    const auto participantIt = simulationIt->second.participants.find(request.participantName);
    if (participantIt == simulationIt->second.participants.end())
    {
        return reply;
    }

    if (!participantIt->second.configurationJson.has_value())
    {
        reply.status = Mgmt::ParticipantConfigurationStatus::NotShared;
        return reply;
    }

    reply.status = Mgmt::ParticipantConfigurationStatus::Available;
    reply.configurationJson = participantIt->second.configurationJson.value();
    return reply;
}

auto RegistryManagementParticipant::MakeSimulations() const -> Mgmt::Simulations
{
    Mgmt::Simulations simulations;
    simulations.sequence = _sequence;
    for (const auto& simulation : _simulations)
    {
        Mgmt::SimulationInfo info;
        info.name = simulation.first;
        for (const auto& participant : simulation.second.participants)
        {
            info.participantNames.push_back(participant.first);
        }
        info.requiredParticipantNames = simulation.second.requiredParticipantNames;
        simulations.simulations.push_back(std::move(info));
    }
    return simulations;
}

void RegistryManagementParticipant::PublishSimulations()
{
    ++_sequence;

    // Publish only hands the message to the I/O thread of the management participant, so it is safe under the lock.
    if (_simulationsPublisher != nullptr)
    {
        _simulationsPublisher->Publish(Mgmt::Serialize(MakeSimulations()));
    }
}

} // namespace VSilKit
