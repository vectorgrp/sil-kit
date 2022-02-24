// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriber.hpp"
#include "IServiceDiscovery.hpp"
#include "YamlParser.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriber::DataSubscriber(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider,
                               const std::string& topic, DataExchangeFormat dataExchangeFormat, const std::map<std::string, std::string>& labels,
                               DataHandlerT defaultDataHandler, NewDataSourceHandlerT newDataSourceHandler)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
    , _topic{topic}
    , _dataExchangeFormat{dataExchangeFormat}
    , _labels{labels}
    , _defaultDataHandler{defaultDataHandler}
    , _newDataSourceHandler{newDataSourceHandler}
{

}

void DataSubscriber::RegisterServiceDiscovery()
{
    _comAdapter->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
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
                DataExchangeFormat pubDataExchangeFormat{ getVal(mw::service::supplKeyDataPublisherPubDxf)};
                auto pubUUID = getVal(mw::service::supplKeyDataPublisherPubUUID);
                std::string labelsStr = getVal(mw::service::supplKeyDataPublisherPubLabels);
                std::map<std::string, std::string> publisherLabels = ib::cfg::Deserialize<std::map<std::string, std::string>>(labelsStr);

                if (topic == _topic && Match(_dataExchangeFormat, pubDataExchangeFormat) &&
                    MatchLabels(_labels, publisherLabels))
                {
                    // NB: The internal subscriber carries its publisher's information
                    // that AssignSpecificDataHandlers() needs to check matching between 
                    // user given dxf/labels and the publisher's dxf/labels.
                    AddInternalSubscriber(pubUUID, pubDataExchangeFormat, publisherLabels);

                    if (_newDataSourceHandler)
                    {
                        _newDataSourceHandler(this, topic, pubDataExchangeFormat, publisherLabels);
                    }
                    // NB: Try to assign specific handlers here as _internalSubscibers has changed
                    AssignSpecificDataHandlers();
                }
            }
        });
}

void DataSubscriber::SetDefaultReceiveMessageHandler(DataHandlerT callback)
{
    for (auto* internalSubscriber : _internalSubscibers)
    {
        internalSubscriber->SetDefaultReceiveMessageHandler(callback);
    }
}


void DataSubscriber::RegisterSpecificDataHandler(const DataExchangeFormat& dataExchangeFormat,
                                                 const std::map<std::string, std::string>& labels,
                                                 DataHandlerT specificDataHandler)
{
    _specificDataHandling.push_back({ _specificDataHandlerId++, dataExchangeFormat, labels, specificDataHandler });
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
                if (Match(dataHandling.dataExchangeFormat, internalSubscriber->GetDataExchangeFormat()) &&
                    MatchLabels(dataHandling.labels, internalSubscriber->GetLabels()))
                {
                    dataHandling.registeredInternalSubscribers.insert(internalSubscriber);
                    internalSubscriber->RegisterSpecificDataHandlerInternal(dataHandling.specificDataHandler);
                }
            }
        }
    }
}

void DataSubscriber::AddInternalSubscriber(const std::string& pubUUID, DataExchangeFormat joinedDataExchangFormat,
                                           const std::map<std::string, std::string>& publisherLabels)
{
    auto internalSubscriber = dynamic_cast<DataSubscriberInternal*>(_comAdapter->CreateDataSubscriberInternal(
        _topic, pubUUID, joinedDataExchangFormat, publisherLabels, _defaultDataHandler, this));
    
    _internalSubscibers.push_back(internalSubscriber);
}

void DataSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
