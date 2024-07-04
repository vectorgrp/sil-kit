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

#include "SilKitToOatppMapper.hpp"

#include "YamlParser.hpp"

#include <string>
#include <type_traits>

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

auto GetSupplementalDataValue(const Core::ServiceDescriptor& serviceDescriptor, const std::string& key) -> oatpp::String
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
    auto value = GetSupplementalDataValue(serviceDescriptor, key);
    return ToUInt64(*value.get());
}

} // namespace

oatpp::Object<SimulationConfigurationDto> CreateSimulationConfigurationDto(const std::string& connectUri)
{
    auto configuration = SimulationConfigurationDto::createShared();
    configuration->connectUri = connectUri;
    return configuration;
}

oatpp::Object<SimulationCreationRequestDto> SilKitToOatppMapper::CreateSimulationCreationRequestDto(
    const std::string& connectUri, uint64_t start)
{
    auto simulation = SimulationCreationRequestDto::createShared();
    simulation->started = start;
    simulation->configuration = CreateSimulationConfigurationDto(connectUri);
    return simulation;
}

SystemState MapSystemState(Services::Orchestration::SystemState systemState)
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

oatpp::Object<SystemStatusDto> SilKitToOatppMapper::CreateSystemStatusDto(
    Services::Orchestration::SystemState systemState)
{
    auto status = SystemStatusDto::createShared();
    status->state = MapSystemState(systemState);
    return status;
}

ParticipantState MapParticipantState(Services::Orchestration::ParticipantState state)
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

oatpp::Object<ParticipantStatusDto> SilKitToOatppMapper::CreateParticipantStatusDto(
    const Services::Orchestration::ParticipantStatus& participantStatus)
{
    auto status = ParticipantStatusDto::createShared();
    status->state = MapParticipantState(participantStatus.state);
    status->enterReason = participantStatus.enterReason;
    status->enterTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(participantStatus.enterTime.time_since_epoch()).count();
    return status;
}

LabelKind MapLabelKind(Services::MatchingLabel::Kind labelKind)
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

oatpp::Object<MatchingLabelDto> CreateMatchingLabelDto(const Services::MatchingLabel& matchingLabel)
{
    auto label = oatpp::Object<MatchingLabelDto>::createShared();
    label->key = matchingLabel.key;
    label->value = matchingLabel.value;
    label->kind = MapLabelKind(matchingLabel.kind);
    return label;
}

oatpp::Vector<oatpp::Object<MatchingLabelDto>> CreateMatchingLabels(const Core::ServiceDescriptor& serviceDescriptor,
                                                                    const std::string& labelsKey)
{
    auto labels = oatpp::Vector<oatpp::Object<MatchingLabelDto>>::createShared();
    std::string labelsStr;
    if (!serviceDescriptor.GetSupplementalDataItem(labelsKey, labelsStr))
    {
        throw SilKitError{"Missing key " + labelsKey + " in supplementalData"};
    }
    std::vector<Services::MatchingLabel> matchingLabels =
        Config::Deserialize<std::vector<Services::MatchingLabel>>(labelsStr);
    std::vector<Services::MatchingLabel>::iterator it;
    for (it = matchingLabels.begin(); it != matchingLabels.end(); it++)
    {
        labels->push_back(CreateMatchingLabelDto(*it));
    }
    return labels;
}

oatpp::Object<ServiceDto> SilKitToOatppMapper::CreateServiceDto(const Core::ServiceDescriptor& serviceDescriptor)
{
    auto controller = ServiceDto::createShared();
    controller->name = serviceDescriptor.GetServiceName();
    controller->networkName = serviceDescriptor.GetNetworkName();
    return controller;
}

oatpp::Object<DataSpecDto> CreateDataSpecDto(const Core::ServiceDescriptor& serviceDescriptor,
                                             const std::string& topicKey, const std::string& mediaTypeKey,
                                             const std::string& labelsKey)
{
    auto dataSpec = DataSpecDto::createShared();
    dataSpec->topic = GetSupplementalDataValue(serviceDescriptor, topicKey);
    dataSpec->mediaType = GetSupplementalDataValue(serviceDescriptor, mediaTypeKey);
    dataSpec->labels = CreateMatchingLabels(serviceDescriptor, labelsKey);
    return dataSpec;
}

oatpp::Object<DataPublisherDto> SilKitToOatppMapper::CreateDataPublisherDto(
    const Core::ServiceDescriptor& serviceDescriptor)
{
    auto dataPublisher = DataPublisherDto::createShared();
    dataPublisher->name = serviceDescriptor.GetServiceName();
    dataPublisher->networkName = serviceDescriptor.GetNetworkName();
    dataPublisher->spec = CreateDataSpecDto(serviceDescriptor, Core::Discovery::supplKeyDataPublisherTopic,
                                            Core::Discovery::supplKeyDataPublisherMediaType,
                                            Core::Discovery::supplKeyDataPublisherPubLabels);
    return dataPublisher;
}

oatpp::Object<DataSubscriberDto> SilKitToOatppMapper::CreateDataSubscriberDto(
    const Core::ServiceDescriptor& serviceDescriptor)
{
    auto dataSubscriber = DataSubscriberDto::createShared();
    dataSubscriber->name = serviceDescriptor.GetServiceName();
    dataSubscriber->spec = CreateDataSpecDto(serviceDescriptor, Core::Discovery::supplKeyDataSubscriberTopic,
                                             Core::Discovery::supplKeyDataSubscriberMediaType,
                                             Core::Discovery::supplKeyDataSubscriberSubLabels);
    return dataSubscriber;
}

oatpp::Object<RpcSpecDto> CreateRpcSpecDto(const Core::ServiceDescriptor& serviceDescriptor,
                                           const std::string& functionNameKey, const std::string& mediaTypeKey,
                                           const std::string& labelsKey)
{
    auto rpcSpec = RpcSpecDto::createShared();
    rpcSpec->functionName = GetSupplementalDataValue(serviceDescriptor, functionNameKey);
    rpcSpec->mediaType = GetSupplementalDataValue(serviceDescriptor, mediaTypeKey);
    rpcSpec->labels = CreateMatchingLabels(serviceDescriptor, labelsKey);
    return rpcSpec;
}

oatpp::Object<RpcClientDto> SilKitToOatppMapper::CreateRpcClientDto(const Core::ServiceDescriptor& serviceDescriptor)
{
    auto rpcClient = RpcClientDto::createShared();
    rpcClient->name = serviceDescriptor.GetServiceName();
    rpcClient->networkName = serviceDescriptor.GetNetworkName();
    rpcClient->spec =
        CreateRpcSpecDto(serviceDescriptor, Core::Discovery::supplKeyRpcClientFunctionName,
                         Core::Discovery::supplKeyRpcClientMediaType, Core::Discovery::supplKeyRpcClientLabels);
    return rpcClient;
}

oatpp::Object<RpcServerDto> SilKitToOatppMapper::CreateRpcServerDto(const Core::ServiceDescriptor& serviceDescriptor)
{
    auto rpcServer = RpcServerDto::createShared();
    rpcServer->name = serviceDescriptor.GetServiceName();
    rpcServer->spec =
        CreateRpcSpecDto(serviceDescriptor, Core::Discovery::supplKeyRpcServerFunctionName,
                         Core::Discovery::supplKeyRpcServerMediaType, Core::Discovery::supplKeyRpcServerLabels);
    return rpcServer;
}

oatpp::Object<SimulationEndDto> SilKitToOatppMapper::CreateSimulationEndDto(uint64_t stop)
{
    auto simulationEnd = SimulationEndDto::createShared();
    simulationEnd->stopped = stop;
    return simulationEnd;
}

auto SilKitToOatppMapper::CreateBulkControllerDto(const ServiceDescriptor& serviceDescriptor)
    -> Object<BulkControllerDto>
{
    auto dto = BulkControllerDto::createShared();

    dto->id = serviceDescriptor.GetServiceId();
    dto->name = serviceDescriptor.GetServiceName();
    dto->networkName = serviceDescriptor.GetNetworkName();

    return dto;
}

auto SilKitToOatppMapper::CreateBulkDataServiceDto(const ServiceDescriptor& serviceDescriptor)
    -> Object<BulkDataServiceDto>
{
    auto dto = BulkDataServiceDto::createShared();

    dto->id = serviceDescriptor.GetServiceId();
    dto->name = serviceDescriptor.GetServiceName();
    dto->networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriber)
    {
        dto->spec = CreateDataSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyDataSubscriberTopic,
                                      SilKit::Core::Discovery::supplKeyDataSubscriberMediaType,
                                      SilKit::Core::Discovery::supplKeyDataSubscriberSubLabels);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataPublisher)
    {
        dto->spec = CreateDataSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyDataPublisherTopic,
                                      SilKit::Core::Discovery::supplKeyDataPublisherMediaType,
                                      SilKit::Core::Discovery::supplKeyDataPublisherPubLabels);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto SilKitToOatppMapper::CreateBulkRpcServiceDto(const ServiceDescriptor& serviceDescriptor)
    -> Object<BulkRpcServiceDto>
{
    auto dto = BulkRpcServiceDto::createShared();

    dto->id = serviceDescriptor.GetServiceId();
    dto->name = serviceDescriptor.GetServiceName();
    dto->networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeRpcClient)
    {
        dto->spec = CreateRpcSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcClientFunctionName,
                                     SilKit::Core::Discovery::supplKeyRpcClientMediaType,
                                     SilKit::Core::Discovery::supplKeyRpcClientLabels);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServer)
    {
        dto->spec = CreateRpcSpecDto(serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcServerFunctionName,
                                     SilKit::Core::Discovery::supplKeyRpcServerMediaType,
                                     SilKit::Core::Discovery::supplKeyRpcServerLabels);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto SilKitToOatppMapper::CreateBulkServiceInternalDto(const ServiceDescriptor& serviceDescriptor)
    -> Object<BulkServiceInternalDto>
{
    auto dto = BulkServiceInternalDto::createShared();

    dto->id = serviceDescriptor.GetServiceId();
    dto->name = serviceDescriptor.GetServiceName();
    dto->networkName = serviceDescriptor.GetNetworkName();

    const auto controllerType = GetControllerType(serviceDescriptor);
    if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        dto->parentId = GetSupplementalDataValueAsEndpointId(
            serviceDescriptor, SilKit::Core::Discovery::supplKeyDataSubscriberInternalParentServiceID);
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServerInternal)
    {
        dto->parentId = GetSupplementalDataValueAsEndpointId(
            serviceDescriptor, SilKit::Core::Discovery::supplKeyRpcServerInternalParentServiceID);
    }
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }

    return dto;
}

auto SilKitToOatppMapper::CreateBulkSimulationDto(const DashboardBulkUpdate& bulkUpdate) -> Object<BulkSimulationDto>
{
    auto bulkSimulationDto = BulkSimulationDto::CreateEmpty();

    if (bulkUpdate.stopped)
    {
        bulkSimulationDto->stopped = static_cast<std::int64_t>(*bulkUpdate.stopped);
    }

    for (const auto& systemState : bulkUpdate.systemStates)
    {
        bulkSimulationDto->system->statuses->emplace_back(CreateSystemStatusDto(systemState));
    }

    std::unordered_map<std::string, BulkParticipantDto::Wrapper> nameToBulkParticipantDto;

    const auto getOrCreateParticipantDto = [&nameToBulkParticipantDto](const std::string& name) -> BulkParticipantDto& {
        auto it = nameToBulkParticipantDto.find(name);
        if (it == nameToBulkParticipantDto.end())
        {
            auto dto = BulkParticipantDto::CreateEmpty();
            dto->name = name;

            it = nameToBulkParticipantDto.emplace(name, std::move(dto)).first;
        }
        return *it->second.get();
    };

    for (const auto& participantConnectionInformation : bulkUpdate.participantConnectionInformations)
    {
        (void)getOrCreateParticipantDto(participantConnectionInformation.participantName);
    }

    for (const auto& participantStatus : bulkUpdate.participantStatuses)
    {
        auto& dto = getOrCreateParticipantDto(participantStatus.participantName);
        dto.statuses->emplace_back(CreateParticipantStatusDto(participantStatus));
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
        bulkSimulationDto->participants->emplace_back(pair.second);
    }

    return bulkSimulationDto;
}

// SilKitToOatppMapper Private Methods

void SilKitToOatppMapper::ProcessServiceDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor)
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

void SilKitToOatppMapper::ProcessControllerDiscovery(BulkParticipantDto& dto,
                                                     const ServiceDescriptor& serviceDescriptor)
{
    const auto controllerType = GetControllerType(serviceDescriptor);

    // Bus Controllers
    if (controllerType == SilKit::Core::Discovery::controllerTypeCan)
    {
        dto.canControllers->emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeEthernet)
    {
        dto.ethernetControllers->emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeFlexray)
    {
        dto.flexrayControllers->emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeLin)
    {
        dto.linControllers->emplace_back(CreateBulkControllerDto(serviceDescriptor));
    }
    // PubSub Services
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataPublisher)
    {
        dto.dataPublishers->emplace_back(CreateBulkDataServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriber)
    {
        dto.dataSubscribers->emplace_back(CreateBulkDataServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeDataSubscriberInternal)
    {
        dto.dataSubscriberInternals->emplace_back(CreateBulkServiceInternalDto(serviceDescriptor));
    }
    // RPC Services
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcClient)
    {
        dto.rpcClients->emplace_back(CreateBulkRpcServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServer)
    {
        dto.rpcServers->emplace_back(CreateBulkRpcServiceDto(serviceDescriptor));
    }
    else if (controllerType == SilKit::Core::Discovery::controllerTypeRpcServerInternal)
    {
        dto.rpcServerInternals->emplace_back(CreateBulkServiceInternalDto(serviceDescriptor));
    }
    // Everything Else
    else
    {
        throw SilKitError{"Unexpected controller type " + controllerType};
    }
}

void SilKitToOatppMapper::ProcessLinkDiscovery(BulkParticipantDto& dto, const ServiceDescriptor& serviceDescriptor)
{
    switch (serviceDescriptor.GetNetworkType())
    {
    case SilKit::Config::NetworkType::CAN:
        dto.canNetworks->emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::Ethernet:
        dto.ethernetNetworks->emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::FlexRay:
        dto.flexrayNetworks->emplace_back(serviceDescriptor.GetNetworkName());
        break;
    case SilKit::Config::NetworkType::LIN:
        dto.linNetworks->emplace_back(serviceDescriptor.GetNetworkName());
        break;
    default:
        break;
    }
}


} // namespace Dashboard
} // namespace SilKit
