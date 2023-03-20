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

namespace SilKit {
namespace Dashboard {

using namespace std::chrono_literals;

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
    case Services::Orchestration::SystemState::Invalid: return SystemState::Invalid;
    case Services::Orchestration::SystemState::ServicesCreated: return SystemState::ServicesCreated;
    case Services::Orchestration::SystemState::CommunicationInitializing: return SystemState::CommunicationInitializing;
    case Services::Orchestration::SystemState::CommunicationInitialized: return SystemState::CommunicationInitialized;
    case Services::Orchestration::SystemState::ReadyToRun: return SystemState::ReadyToRun;
    case Services::Orchestration::SystemState::Running: return SystemState::Running;
    case Services::Orchestration::SystemState::Paused: return SystemState::Paused;
    case Services::Orchestration::SystemState::Stopping: return SystemState::Stopping;
    case Services::Orchestration::SystemState::Stopped: return SystemState::Stopped;
    case Services::Orchestration::SystemState::Error: return SystemState::Error;
    case Services::Orchestration::SystemState::ShuttingDown: return SystemState::ShuttingDown;
    case Services::Orchestration::SystemState::Shutdown: return SystemState::Shutdown;
    case Services::Orchestration::SystemState::Aborting: return SystemState::Aborting;
    default: throw SilKitError{"Incomplete system state mapping"};
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
    case Services::Orchestration::ParticipantState::Invalid: return ParticipantState::Invalid;
    case Services::Orchestration::ParticipantState::ServicesCreated: return ParticipantState::ServicesCreated;
    case Services::Orchestration::ParticipantState::CommunicationInitializing:
        return ParticipantState::CommunicationInitializing;
    case Services::Orchestration::ParticipantState::CommunicationInitialized:
        return ParticipantState::CommunicationInitialized;
    case Services::Orchestration::ParticipantState::ReadyToRun: return ParticipantState::ReadyToRun;
    case Services::Orchestration::ParticipantState::Running: return ParticipantState::Running;
    case Services::Orchestration::ParticipantState::Paused: return ParticipantState::Paused;
    case Services::Orchestration::ParticipantState::Stopping: return ParticipantState::Stopping;
    case Services::Orchestration::ParticipantState::Stopped: return ParticipantState::Stopped;
    case Services::Orchestration::ParticipantState::Error: return ParticipantState::Error;
    case Services::Orchestration::ParticipantState::ShuttingDown: return ParticipantState::ShuttingDown;
    case Services::Orchestration::ParticipantState::Shutdown: return ParticipantState::Shutdown;
    case Services::Orchestration::ParticipantState::Aborting: return ParticipantState::Aborting;
    default: throw SilKitError{"Incomplete participant state mapping"};
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
    case Services::MatchingLabel::Kind::Optional: return LabelKind::Optional;
    case Services::MatchingLabel::Kind::Mandatory: return LabelKind::Mandatory;
    default: throw SilKitError{"Incomplete mapping"};
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

oatpp::String GetSupplementalDataValue(const Core::ServiceDescriptor& serviceDescriptor, const std::string& key)
{
    std::string str;
    if (!serviceDescriptor.GetSupplementalDataItem(key, str))
    {
        throw SilKitError{"Missing key " + key + " in supplementalData"};
    }
    return str;
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

} // namespace Dashboard
} // namespace SilKit
