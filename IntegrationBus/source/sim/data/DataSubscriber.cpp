// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriber.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriber::DataSubscriber(mw::IComAdapterInternal* comAdapter, cfg::DataPort config, mw::sync::ITimeProvider* timeProvider, CallbackExchangeFormatT callback)
  : _comAdapter{comAdapter}, _timeProvider{timeProvider}, _callback{callback}
{
    _config = std::move(config);
}

auto DataSubscriber::Config() const -> const cfg::DataPort&
{
    return _config;
}

void DataSubscriber::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const PublisherAnnouncement& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(msg);
}

void DataSubscriber::ReceiveMessage(const PublisherAnnouncement& msg)
{
    if (msg.topic == _config.name && Match(msg.pubDataExchangeFormat, _config.dataExchangeFormat))
    {
        AddInternalSubscriber(msg.pubUUID, Join(msg.pubDataExchangeFormat, _config.dataExchangeFormat));
    }
}

void DataSubscriber::SetReceiveMessageHandler(CallbackExchangeFormatT callback)
{
    for (auto* internalSubscriber : _internalSubscibers)
    {
        internalSubscriber->SetReceiveMessageHandler(callback);
    }
}

void DataSubscriber::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}

auto DataSubscriber::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}


void DataSubscriber::AddInternalSubscriber(const std::string& linkName, DataExchangeFormat joinedDataExchangFormat)
{
    auto internalSubscriber = dynamic_cast<DataSubscriberInternal*>(_comAdapter->CreateDataSubscriberInternal(
        _config.name, linkName, joinedDataExchangFormat, _callback));
    
    _internalSubscibers.push_back(internalSubscriber);
}

void DataSubscriber::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace data
} // namespace sim
} // namespace ib
