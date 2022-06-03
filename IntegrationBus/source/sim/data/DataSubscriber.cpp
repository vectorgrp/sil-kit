// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriber.hpp"
#include "IServiceDiscovery.hpp"
#include "YamlParser.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriber::DataSubscriber(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
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
        [this](ib::mw::service::ServiceDiscoveryEvent::Type discoveryType,
               const ib::mw::ServiceDescriptor& serviceDescriptor) {

                auto getVal = [serviceDescriptor](std::string key) {
                    std::string tmp;
                    if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
                    { 
                        throw std::runtime_error{"Unknown key in supplementalData"}; 
                    }
                    return tmp;
                };

                auto topic = getVal(mw::service::supplKeyDataPublisherTopic);
                std::string pubMediaType{ getVal(mw::service::supplKeyDataPublisherMediaType)};
                auto pubUUID = getVal(mw::service::supplKeyDataPublisherPubUUID);
                std::string labelsStr = getVal(mw::service::supplKeyDataPublisherPubLabels);
                std::map<std::string, std::string> publisherLabels = ib::cfg::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (topic == _topic && MatchMediaType(_mediaType, pubMediaType) &&
                    MatchLabels(_labels, publisherLabels))
                {
                    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

                    if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated)
                    {
                        // NB: The internal subscriber carries its publisher's information
                        // that AssignSpecificDataHandlers() needs to check matching between
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
                        AssignSpecificDataHandlers();
                    }
                    else if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceRemoved)
                    {
                        RemoveInternalSubscriber(pubUUID);
                    }
                }
            
        }, mw::service::controllerTypeDataPublisher, _topic);
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

void DataSubscriber::AddExplicitDataMessageHandler(DataMessageHandlerT specificDataHandler,
                                                   const std::string& mediaType,
                                                   const std::map<std::string, std::string>& labels)
{
    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

    _specificDataHandling.push_back({ _specificDataHandlerId++, mediaType, labels, specificDataHandler, {} });
    // NB: Try to assign specific handlers here as _specificDataHandling has changed
    AssignSpecificDataHandlers();
}

void DataSubscriber::AssignSpecificDataHandlers()
{
    for (auto internalSubscriber : _internalSubscribers)
    {
        for (auto& dataHandling : _specificDataHandling)
        {
            // Register a specificDataHandler only once per internalSubscriber
            auto it = dataHandling.registeredInternalSubscribers.find(internalSubscriber.second);
            if (it == dataHandling.registeredInternalSubscribers.end())
            {
                if (MatchMediaType(dataHandling.mediaType, internalSubscriber.second->GetMediaType())
                    && MatchLabels(dataHandling.labels, internalSubscriber.second->GetLabels()))
                {
                    dataHandling.registeredInternalSubscribers.insert(internalSubscriber.second);
                    internalSubscriber.second->RegisterSpecificDataHandlerInternal(dataHandling.specificDataHandler);
                }
            }
        }
    }
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
        for (auto& dataHandling : _specificDataHandling)
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

void DataSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
