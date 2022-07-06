// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "silkit/core/fwd_decl.hpp"
#include "silkit/services/pubsub/IDataSubscriber.hpp"
#include "ITimeConsumer.hpp"

#include "IMsgForDataSubscriber.hpp"
#include "IParticipantInternal.hpp"
#include "DataSubscriberInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriber
    : public IDataSubscriber
    , public IMsgForDataSubscriber
    , public Core::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    DataSubscriber(Core::IParticipantInternal* participant, Core::Orchestration::ITimeProvider* timeProvider, const std::string& topic,
                   const std::string& mediaType, const std::map<std::string, std::string>& labels,
                   DataMessageHandlerT defaultDataHandler, NewDataPublisherHandlerT newDataSourceHandler);

public:
    void RegisterServiceDiscovery();

    void SetDefaultDataMessageHandler(DataMessageHandlerT callback) override;

    auto AddExplicitDataMessageHandler(DataMessageHandlerT dataMessageHandler, const std::string& mediaType,
                                       const std::map<std::string, std::string>& labels) -> HandlerId override;

    void RemoveExplicitDataMessageHandler(HandlerId handlerId) override;

    // SilKit::Core::Orchestration::ITimeConsumer
    inline void SetTimeProvider(Core::Orchestration::ITimeProvider* provider) override;
    
    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    void AddInternalSubscriber(const std::string& pubUUID, const std::string& joinedMediaType,
        const std::map<std::string, std::string>& publisherLabels);

    void RemoveInternalSubscriber(const std::string& pubUUID);

    void AddExplicitDataHandlersToInternalSubscribers();

private:
    std::string _topic;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    DataMessageHandlerT _defaultDataHandler;
    NewDataPublisherHandlerT _newDataSourceHandler;

    Core::ServiceDescriptor _serviceDescriptor{};

    std::underlying_type_t<HandlerId> _nextExplicitDataMessageHandlerId = 0;
    std::unordered_map<std::string, DataSubscriberInternal*> _internalSubscribers;
    std::vector<ExplicitDataMessageHandlerInfo> _explicitDataMessageHandlers;

    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};

    std::unordered_set<SourceInfo, SourceInfo::HashFunction> _announcedDataSources;

    mutable std::recursive_mutex _internalSubscribersMx;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriber::SetTimeProvider(Core::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

void DataSubscriber::SetServiceDescriptor(const SilKit::Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriber::GetServiceDescriptor() const -> const SilKit::Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
