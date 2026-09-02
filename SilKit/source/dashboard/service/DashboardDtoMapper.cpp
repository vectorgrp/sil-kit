// SPDX-FileCopyrightText: 2022-2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/service/DashboardDtoMapper.hpp"

#include <string>
#include <type_traits>
#include <unordered_map>

#include "config/YamlParser.hpp"
#include "core/internal/traits/SilKitLoggingTraits.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "util/StringHelpers.hpp"

namespace SilKit {
namespace Dashboard {

using namespace std::chrono_literals;

namespace {

constexpr bool u64_is_ul = std::is_same<std::uint64_t, unsigned long>::value;
constexpr bool u64_is_ull = std::is_same<std::uint64_t, unsigned long long>::value;

auto ToUInt64(const std::string& value) -> std::uint64_t
{
    static_assert(u64_is_ul || u64_is_ull, "");

    if (u64_is_ul)
    {
        return std::stoul(value);
    }

    if (u64_is_ull)
    {
        return std::stoull(value);
    }
}

auto GetSupplementalDataValue(const Core::ServiceDescriptor& serviceDescriptor, const std::string& key) -> std::string
{
    std::string str;
    if (!serviceDescriptor.GetSupplementalDataItem(key, str))
    {
        throw SilKitError{"Missing key " + key + " in supplementalData"};
    }
    return str;
}

auto GetControllerType(const SilKit::Core::ServiceDescriptor& serviceDescriptor) -> std::string
{
    return GetSupplementalDataValue(serviceDescriptor, Core::Discovery::controllerType);
}

auto GetSupplementalDataValueAsEndpointId(const SilKit::Core::ServiceDescriptor& serviceDescriptor,
                                          const std::string& key) -> SilKit::Core::EndpointId
{
    return ToUInt64(GetSupplementalDataValue(serviceDescriptor, key));
}

auto MapSystemState(Services::Orchestration::SystemState systemState) -> SystemState
{
    switch (systemState)
    {
    case Services::Orchestration::SystemState::Invalid:
        return SystemState::Invalid;
    case Services::Orchestration::SystemState::ServicesCreated:
        return SystemState::ServicesCreated;
    case Services::Orchestration::SystemState::CommunicationInitializing:
        return SystemState::CommunicationInitializing;
    case Services::Orchestration::SystemState::CommunicationInitialized:
        return SystemState::CommunicationInitialized;
    case Services::Orchestration::SystemState::ReadyToRun:
        return SystemState::ReadyToRun;
    case Services::Orchestration::SystemState::Running:
        return SystemState::Running;
    case Services::Orchestration::SystemState::Paused:
        return SystemState::Paused;
    case Services::Orchestration::SystemState::Stopping:
        return SystemState::Stopping;
    case Services::Orchestration::SystemState::Stopped:
        return SystemState::Stopped;
    case Services::Orchestration::SystemState::Error:
        return SystemState::Error;
    case Services::Orchestration::SystemState::ShuttingDown:
        return SystemState::ShuttingDown;
    case Services::Orchestration::SystemState::Shutdown:
        return SystemState::Shutdown;
    case Services::Orchestration::SystemState::Aborting:
        return SystemState::Aborting;
    default:
        throw SilKitError{"Incomplete system state mapping"};
    }
}

auto MapParticipantState(Services::Orchestration::ParticipantState state) -> ParticipantState
{
    switch (state)
    {
    case Services::Orchestration::ParticipantState::Invalid:
        return ParticipantState::Invalid;
    case Services::Orchestration::ParticipantState::ServicesCreated:
        return ParticipantState::ServicesCreated;
    case Services::Orchestration::ParticipantState::CommunicationInitializing:
        return ParticipantState::CommunicationInitializing;
    case Services::Orchestration::ParticipantState::CommunicationInitialized:
        return ParticipantState::CommunicationInitialized;
    case Services::Orchestration::ParticipantState::ReadyToRun:
        return ParticipantState::ReadyToRun;
    case Services::Orchestration::ParticipantState::Running:
        return ParticipantState::Running;
    case Services::Orchestration::ParticipantState::Paused:
        return ParticipantState::Paused;
    case Services::Orchestration::ParticipantState::Stopping:
        return ParticipantState::Stopping;
    case Services::Orchestration::ParticipantState::Stopped:
        return ParticipantState::Stopped;
    case Services::Orchestration::ParticipantState::Error:
        return ParticipantState::Error;
    case Services::Orchestration::ParticipantState::ShuttingDown:
        return ParticipantState::ShuttingDown;
    case Services::Orchestration::ParticipantState::Shutdown:
        return ParticipantState::Shutdown;
    case Services::Orchestration::ParticipantState::Aborting:
        return ParticipantState::Aborting;
    default:
        throw SilKitError{"Incomplete participant state mapping"};
    }
}

auto MapLabelKind(Services::MatchingLabel::Kind labelKind) -> LabelKind
{
    switch (labelKind)
    {
    case Services::MatchingLabel::Kind::Optional:
        return LabelKind::Optional;
    case Services::MatchingLabel::Kind::Mandatory:
        return LabelKind::Mandatory;
    default:
        throw SilKitError{"Incomplete mapping"};
    }
}

auto CreateMatchingLabelDto(const Services::MatchingLabel& matchingLabel) -> MatchingLabelDto
{
    MatchingLabelDto label{};
    label.key = matchingLabel.key;
    label.value = matchingLabel.value;
    label.kind = MapLabelKind(matchingLabel.kind);
    return label;
}

auto CreateMatchingLabels(const Core::ServiceDescriptor& serviceDescriptor, const std::string& labelsKey)
    -> std::vector<MatchingLabelDto>
{
    std::string labelsStr;
    if (!serviceDescriptor.GetSupplementalDataItem(labelsKey, labelsStr))
    {
        throw SilKitError{"Missing key " + labelsKey + " in supplementalData"};
    }

    std::vector<MatchingLabelDto> labels;
    for (const auto& matchingLabel : Config::Deserialize<std::vector<Services::MatchingLabel>>(labelsStr))
    {
        labels.emplace_back(CreateMatchingLabelDto(matchingLabel));
    }
    return labels;
}

auto CreateDataSpecDto(const Core::ServiceDescriptor& serviceDescriptor, const std::string& topicKey,
                       const std::string& mediaTypeKey, const std::string& labelsKey) -> DataSpecDto
{
    DataSpecDto dataSpec{};
    dataSpec.topic = GetSupplementalDataValue(serviceDescriptor, topicKey);
    dataSpec.mediaType = GetSupplementalDataValue(serviceDescriptor, mediaTypeKey);
    dataSpec.labels = CreateMatchingLabels(serviceDescriptor, labelsKey);
    return dataSpec;
}

auto CreateRpcSpecDto(const Core::ServiceDescriptor& serviceDescriptor, const std::string& functionNameKey,
                      const std::string& mediaTypeKey, const std::string& labelsKey) -> RpcSpecDto
{
    RpcSpecDto rpcSpec{};
    rpcSpec.functionName = GetSupplementalDataValue(serviceDescriptor, functionNameKey);
    rpcSpec.mediaType = GetSupplementalDataValue(serviceDescriptor, mediaTypeKey);
    rpcSpec.labels = CreateMatchingLabels(serviceDescriptor, labelsKey);
    return rpcSpec;
}

} // namespace

DashboardDtoMapper::DashboardDtoMapper(Services::Logging::ILoggerInternal* logger)
    : _logger{logger}
{
}

auto DashboardDtoMapper::CreateSimulationCreationRequestDto(const std::string& connectUri, uint64_t start)
    -> SimulationCreationRequestDto
{
    SimulationCreationRequestDto simulation{};
    simulation.started = start;
    simulation.configuration.connectUri = connectUri;
    return simulation;
}

auto DashboardDtoMapper::CreateSystemStatusDto(Services::Orchestration::SystemState systemState) -> SystemStatusDto
{
    SystemStatusDto status{};
    status.state = MapSystemState(systemState);
    return status;
}

auto DashboardDtoMapper::CreateParticipantStatusDto(const Services::Orchestration::ParticipantStatus& participantStatus)
    -> ParticipantStatusDto
{
    ParticipantStatusDto status{};
    status.state = MapParticipantState(participantStatus.state);
    status.enterReason = participantStatus.enterReason;
    status.enterTime = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(participantStatus.enterTime.time_since_epoch()).count());
    return status;
}

auto DashboardDtoMapper::CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor) -> BulkControllerDto
{
    BulkControllerDto dto{};

    dto.id = serviceDescriptor.GetServiceId();
    dto.name = serviceDescriptor.GetServiceName();
    dto.networkName = serviceDescriptor.GetNetworkName();

    return dto;
}

auto DashboardDtoMapper::CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor) -> BulkDataServiceDto
{
    BulkDataServiceDto dto{};

    dto.id = serviceDescriptor.GetServiceId();
    dto.name = serviceDescriptor.GetServiceName();
    dto.networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriber)
    {
        dto.spec = CreateDataSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyDataSubscriberTopic,
                                     SilKit::Core::Discovery::supplKeyDataSubscriberMediaType,
                                     SilKit::Core::Discovery::supplKeyDataSubscriberSubLabels);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataPublisher)
    {
        dto.spec = CreateDataSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyDataPublisherTopic,
                                     SilKit::Core::Discovery::supplKeyDataPublisherMediaType,
                                     SilKit::Core::Discovery::supplKeyDataPublisherPubLabels);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto DashboardDtoMapper::CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor) -> BulkRpcServiceDto
{
    BulkRpcServiceDto dto{};

    dto.id = serviceDescriptor.GetServiceId();
    dto.name = serviceDescriptor.GetServiceName();
    dto.networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeRpcClient)
    {
        dto.spec = CreateRpcSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcClientFunctionName,
                                    SilKit::Core::Discovery::supplKeyRpcClientMediaType,
                                    SilKit::Core::Discovery::supplKeyRpcClientLabels);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServer)
    {
        dto.spec = CreateRpcSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcServerFunctionName,
                                    SilKit::Core::Discovery::supplKeyRpcServerMediaType,
                                    SilKit::Core::Discovery::supplKeyRpcServerLabels);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto DashboardDtoMapper::CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
    -> BulkServiceInternalDto
{
    BulkServiceInternalDto dto{};

    dto.id = serviceDescriptor.GetServiceId();
    dto.name = serviceDescriptor.GetServiceName();
    dto.networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        dto.parentId = GetSupplementalDataValueAsEndpointId(
            serviceDescriptor, SilKit::Core::Discovery::supplKeyDataSubscriberInternalParentServiceID);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServerInternal)
    {
        dto.parentId = GetSupplementalDataValueAsEndpointId(
            serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcServerInternalParentServiceID);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto DashboardDtoMapper::CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> BulkSimulationDto
{
    BulkSimulationDto bulkSimulationDto{};

    if (bulkUpdate.stopped)
    {
        bulkSimulationDto.stopped = static_cast<std::int64_t>(*bulkUpdate.stopped);
    }

    for (const auto& systemState : bulkUpdate.systemStates)
    {
        bulkSimulationDto.system.statuses.emplace_back(CreateSystemStatusDto(systemState));
    }

    std::unordered_map<std::string, BulkParticipantDto> nameToBulkParticipantDto;

    const auto getOrCreateParticipantDto = [&nameToBulkParticipantDto](const std::string& name) -> BulkParticipantDto& {
        auto it = nameToBulkParticipantDto.find(name);
        if (it == nameToBulkParticipantDto.end())
        {
            BulkParticipantDto dto{};
            dto.name = name;

            it = nameToBulkParticipantDto.emplace(name, std::move(dto)).first;
        }
        return it->second;
    };

    for (const auto& participantConnectionInformation : bulkUpdate.participantConnectionInformations)
    {
        (void)getOrCreateParticipantDto(participantConnectionInformation.participantName);
    }

    for (const auto& participantStatus : bulkUpdate.participantStatuses)
    {
        auto& dto = getOrCreateParticipantDto(participantStatus.participantName);
        dto.statuses.emplace_back(CreateParticipantStatusDto(participantStatus));
    }

    for (const auto& serviceData : bulkUpdate.serviceDatas)
    {
        if (serviceData.discoveryType != Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
        {
            continue;
        }

        const auto& serviceDescriptor = serviceData.serviceDescriptor;
        auto& dto = getOrCreateParticipantDto(serviceDescriptor.GetParticipantName());

        ProcessServiceDiscovery(dto, serviceDescriptor);
    }

    for (auto& pair : nameToBulkParticipantDto)
    {
        bulkSimulationDto.participants.emplace_back(std::move(pair.second));
    }

    return bulkSimulationDto;
}

auto DashboardDtoMapper::CreateMetricsUpdateDto(const std::string& participantName,
                                                const VSilKit::MetricsUpdate& metricsUpdate) -> MetricsUpdateDto
{
    MetricsUpdateDto dto{};
    for (const auto& metricData : metricsUpdate.metrics)
    {
        auto setValues = [&participantName](auto& dataDto, const auto& metric) {
            dataDto.pn = participantName;
            dataDto.ts = metric.timestamp;
            auto&& nameList = SilKit::Util::SplitString(metric.name, "/");
            std::copy(nameList.begin(), nameList.end(), std::back_inserter(dataDto.mn));
        };

        switch (metricData.kind)
        {
        case VSilKit::MetricKind::COUNTER:
        {
            CounterDataDto dataDto{};
            setValues(dataDto, metricData);
            // MetricsManager formats counters with std::to_string, so this round-trips exactly.
            dataDto.mv = Config::Deserialize<int64_t>(metricData.value);
            dto.counters.emplace_back(std::move(dataDto));
            break;
        }
        case VSilKit::MetricKind::STATISTIC:
        {
            StatisticDataDto dataDto{};
            setValues(dataDto, metricData);
            /* MetricsManager emits shortest-round-trip doubles, which can need 17 significant
             * digits, while the writer re-emits with "%.16g". oatpp did exactly the same
             * parse-then-reformat, so the (slightly lossy) values on the wire do not change. */
            dataDto.mv = Config::Deserialize<std::vector<double>>(metricData.value);
            dto.statistics.emplace_back(std::move(dataDto));
            break;
        }
        case VSilKit::MetricKind::ATTRIBUTE:
        case VSilKit::MetricKind::STRING_LIST:
        {
            AttributeDataDto dataDto{};
            setValues(dataDto, metricData);
            dataDto.mv = metricData.value;
            dto.attributes.emplace_back(std::move(dataDto));
            break;
        }
        default:
            throw SilKit::SilKitError{"MetricsUpdate unknown MetricKind"};
        }
    }
    return dto;
}


// DashboardDtoMapper Private Methods

void DashboardDtoMapper::ProcessServiceDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor)
{
    switch (serviceDescriptor.GetServiceType())
    {
    case SilKit::Core::ServiceType::Controller:
        ProcessControllerDiscovery(dto, serviceDescriptor);
        break;
    case SilKit::Core::ServiceType::Link:
        ProcessLinkDiscovery(dto, serviceDescriptor);
        break;
    default:
        break;
    }
}

void DashboardDtoMapper::ProcessControllerDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor)
{
    const auto controllerType = GetControllerType(serviceDescriptor);

    // Bus Controllers
    if (controllerType == SilKit::Core::Discovery::controllerTypeCan)
    {
        dto.canControllers.emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeEthernet)
    {
        dto.ethernetControllers.emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeFlexray)
    {
        dto.flexrayControllers.emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeLin)
    {
        dto.linControllers.emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    // PubSub Services
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataPublisher)
    {
        dto.dataPublishers.emplace_back(CreateBulkDataServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriber)
    {
        dto.dataSubscribers.emplace_back(CreateBulkDataServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        dto.dataSubscriberInternals.emplace_back(CreateBulkServiceInternalDto(serviceDescriptor));
    }
    // RPC Services
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcClient)
    {
        dto.rpcClients.emplace_back(CreateBulkRpcServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServer)
    {
        dto.rpcServers.emplace_back(CreateBulkRpcServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServerInternal)
    {
        dto.rpcServerInternals.emplace_back(CreateBulkServiceInternalDto(serviceDescriptor));
    }
    // Everything Else
    else
    {
        /* Not a bus or pub/sub service, so there is nothing to show for it. This must not throw:
         * every controller type SIL Kit ever adds would otherwise take down all dashboard
         * reporting for the rest of the process, since the worker cannot resume a batch it failed
         * to map. Report it and move on. */
        if (_logger != nullptr)
        {
            _logger->MakeMessage(Services::Logging::Level::Debug, TopicOf(*this))
                .SetMessage("Dashboard: ignoring service {} of unhandled controller type {}",
                            serviceDescriptor.GetServiceName(), controllerType)
                .Dispatch();
        }
    }
}

void DashboardDtoMapper::ProcessLinkDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor)
{
    switch (serviceDescriptor.GetNetworkType())
    {
    case SilKit::Config::NetworkType::CAN:
        dto.canNetworks.emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::Ethernet:
        dto.ethernetNetworks.emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::FlexRay:
        dto.flexrayNetworks.emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::LIN:
        dto.linNetworks.emplace_back(serviceDescriptor.GetNetworkName());
        break;
    default:
        break;
    }
}


} // namespace Dashboard
} // namespace SilKit
