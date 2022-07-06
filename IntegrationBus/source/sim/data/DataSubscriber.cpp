// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriber.hpp"

#include "IServiceDiscovery.hpp"
#include "YamlParser.hpp"

#include "silkit/core/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataSubscriber::DataSubscriber(Core::IParticipantInternal* participant, Core::Orchestration::ITimeProvider* timeProvider,
                               const std::string& topic, const std::string& mediaType, const std::map<std::string, std::string>& labels,
                               DataMessageHandlerT defaultDataHandler, NewDataPublisherHandlerT newDataSourceHandler)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _defaultDataHandler{defaultDataHandler}
    , _newDataSourceHandler{newDataSourceHandler}
    , _timeProvider{timeProvider}
    , _participant{ participant }
{
}

void DataSubscriber::RegisterServiceDiscovery()
{
    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {

                auto getVal = [serviceDescriptor](std::string key) {
                    std::string tmp;
                    if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                    { 
                        throw std::runtime_error{"Unknown key in supplementalData"}; 
                    }
                    return tmp;
                };

                auto topic = getVal(Core::Discovery::supplKeyDataPublisherTopic);
                std::string pubMediaType{ getVal(Core::Discovery::supplKeyDataPublisherMediaType)};
                auto pubUUID = getVal(Core::Discovery::supplKeyDataPublisherPubUUID);
                std::string labelsStr = getVal(Core::Discovery::supplKeyDataPublisherPubLabels);
                std::map<std::string, std::string> publisherLabels = SilKit::Config::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (topic == _topic && MatchMediaType(_mediaType, pubMediaType) &&
                    MatchLabels(_labels, publisherLabels))
                {
                    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

                    if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                    {
                        // NB: The internal subscriber carries its publisher's information
                        // that AddExplicitDataHandlersToInternalSubscribers() needs to check matching between
                        // user given mediaType/labels and the publisher's mediaType/labels.
                        AddInternalSubscriber(pubUUID, pubMediaType, publisherLabels);
                        
                        if (_newDataSourceHandler)
                        {
                            // Prevent multiple calls to _newDataSourceHandler for same set of 
                            // pubMediaType, labelsStr (topic is always the same)
                            SourceInfo sourceInfo{pubMediaType, publisherLabels};
                            auto it = _announcedDataSources.find(sourceInfo);
                            if (it == _announcedDataSources.end())
                            {
                                _announcedDataSources.emplace(sourceInfo);
                                _newDataSourceHandler(this,
                                                      {_timeProvider->Now(), topic, pubMediaType, publisherLabels});
                            }
                        }
                        // NB: Try to assign specific handlers here as _internalSubscibers has changed
                        AddExplicitDataHandlersToInternalSubscribers();
                    }
                    else if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved)
                    {
                        RemoveInternalSubscriber(pubUUID);
                    }
                }
            
        }, Core::Discovery::controllerTypeDataPublisher, _topic);
}

void DataSubscriber::SetDefaultDataMessageHandler(DataMessageHandlerT callback)
{
    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

    _defaultDataHandler = callback;
    for (auto internalSubscriber : _internalSubscribers)
    {
        internalSubscriber.second->SetDefaultDataMessageHandler(callback);
    }
}

auto DataSubscriber::AddExplicitDataMessageHandler(DataMessageHandlerT dataMessageHandler,
                                                   const std::string& mediaType,
                                                   const std::map<std::string, std::string>& labels) -> HandlerId
{
    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

    const auto handlerId = static_cast<HandlerId>(_nextExplicitDataMessageHandlerId++);

    _explicitDataMessageHandlers.push_back({handlerId, mediaType, labels, dataMessageHandler, {}});
    // NB: Try to assign specific handlers here as _explicitDataMessageHandlers has changed
    AddExplicitDataHandlersToInternalSubscribers();

    return handlerId;
}

void DataSubscriber::RemoveExplicitDataMessageHandler(HandlerId handlerId)
{
    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

    auto it = std::find_if(_explicitDataMessageHandlers.begin(), _explicitDataMessageHandlers.end(),
                           [handlerId](const ExplicitDataMessageHandlerInfo& info) -> bool {
                               return info.id == handlerId;
                           });

    if (it == _explicitDataMessageHandlers.end())
    {
        _participant->GetLogger()->Warn("RemoveExplicitDataMessageHandler failed: Unknown HandlerId.");
        return;
    }

    for (const auto& kv : it->registeredInternalSubscribers)
    {
        const auto& internalSubscriber = kv.first;
        const auto& internalHandlerId = kv.second;
        internalSubscriber->RemoveExplicitDataMessageHandler(internalHandlerId);
    }

    _explicitDataMessageHandlers.erase(it);
}

void DataSubscriber::AddInternalSubscriber(const std::string& pubUUID, const std::string& joinedMediaType,
                                           const std::map<std::string, std::string>& publisherLabels)
{
    auto internalSubscriber = dynamic_cast<DataSubscriberInternal*>(_participant->CreateDataSubscriberInternal(
        _topic, pubUUID, joinedMediaType, publisherLabels, _defaultDataHandler, this));
    
    _internalSubscribers.emplace(pubUUID, internalSubscriber);
}

void DataSubscriber::RemoveInternalSubscriber(const std::string& pubUUID)
{
    auto internalSubscriber = _internalSubscribers.find(pubUUID);
    if (internalSubscriber != _internalSubscribers.end())
    {
        for (auto& dataHandling : _explicitDataMessageHandlers)
        {
            auto it = dataHandling.registeredInternalSubscribers.find(internalSubscriber->second);
            if (it != dataHandling.registeredInternalSubscribers.end())
            {
                dataHandling.registeredInternalSubscribers.erase(it);
            }
        }
        _internalSubscribers.erase(pubUUID);
    }
    // TODO: Actual controller is still alive, need to remove controller added via CreateDataSubscriberInternal
}

void DataSubscriber::AddExplicitDataHandlersToInternalSubscribers()
{
    for (auto internalSubscriber : _internalSubscribers)
    {
        for (auto& dataHandling : _explicitDataMessageHandlers)
        {
            // Register a explicitDataMessageHandler only once per internalSubscriber
            auto it = dataHandling.registeredInternalSubscribers.find(internalSubscriber.second);
            if (it == dataHandling.registeredInternalSubscribers.end())
            {
                if (MatchMediaType(dataHandling.mediaType, internalSubscriber.second->GetMediaType())
                    && MatchLabels(dataHandling.labels, internalSubscriber.second->GetLabels()))
                {
                    const auto internalHandlerId = internalSubscriber.second->AddExplicitDataMessageHandler(
                        dataHandling.explicitDataMessageHandler);
                    dataHandling.registeredInternalSubscribers.emplace(internalSubscriber.second, internalHandlerId);
                }
            }
        }
    }
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
