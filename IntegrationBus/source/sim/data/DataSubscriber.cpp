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
    _participant->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
        [this](ib::mw::service::ServiceDiscoveryEvent::Type discoveryType,
               const ib::mw::ServiceDescriptor& serviceDescriptor) {
            if (discoveryType == ib::mw::service::ServiceDiscoveryEvent::Type::ServiceCreated)
            {
                std::string controllerType;
                if (!(serviceDescriptor.GetSupplementalDataItem(mw::service::controllerType, controllerType) &&
                    controllerType == mw::service::controllerTypeDataPublisher))
                {
                    return;
                }

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
                    // NB: The internal subscriber carries its publisher's information
                    // that AssignSpecificDataHandlers() needs to check matching between 
                    // user given mediaType/labels and the publisher's mediaType/labels.
                    AddInternalSubscriber(pubUUID, pubMediaType, publisherLabels);

                    if (_newDataSourceHandler)
                    {
                        _newDataSourceHandler(this, { _timeProvider->Now(), topic, pubMediaType, publisherLabels });
                    }
                    // NB: Try to assign specific handlers here as _internalSubscibers has changed
                    AssignSpecificDataHandlers();
                }
            }
        });
}

void DataSubscriber::SetDefaultDataMessageHandler(DataMessageHandlerT callback)
{
    for (auto* internalSubscriber : _internalSubscibers)
    {
        internalSubscriber->SetDefaultDataMessageHandler(callback);
    }
}


void DataSubscriber::AddExplicitDataMessageHandler(DataMessageHandlerT specificDataHandler,
                                                   const std::string& mediaType,
                                                   const std::map<std::string, std::string>& labels)
{
    _specificDataHandling.push_back({ _specificDataHandlerId++, mediaType, labels, specificDataHandler, {} });
    // NB: Try to assign specific handlers here as _specificDataHandling has changed
    AssignSpecificDataHandlers();
}

void DataSubscriber::AssignSpecificDataHandlers()
{
    for (auto* internalSubscriber : _internalSubscibers)
    {
        for (auto& dataHandling : _specificDataHandling)
        {
            // Register a specificDataHandler only once per internalSubscriber
            auto it = dataHandling.registeredInternalSubscribers.find(internalSubscriber);
            if (it == dataHandling.registeredInternalSubscribers.end())
            {
                if (MatchMediaType(dataHandling.mediaType, internalSubscriber->GetMediaType()) &&
                    MatchLabels(dataHandling.labels, internalSubscriber->GetLabels()))
                {
                    dataHandling.registeredInternalSubscribers.insert(internalSubscriber);
                    internalSubscriber->RegisterSpecificDataHandlerInternal(dataHandling.specificDataHandler);
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
    
    _internalSubscibers.push_back(internalSubscriber);
}

void DataSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
