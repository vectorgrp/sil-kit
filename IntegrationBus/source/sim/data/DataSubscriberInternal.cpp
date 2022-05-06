// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include <cassert>

#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace data {

DataSubscriberInternal::DataSubscriberInternal(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider, 
                                               const std::string& topic, const std::string& mediaType,
                                               const std::map<std::string, std::string>& labels,
                                               DataMessageHandlerT defaultHandler, IDataSubscriber* parent)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _defaultHandler{defaultHandler}
    , _parent{parent}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
    (void)_participant;
}

void DataSubscriberInternal::SetDefaultDataMessageHandler(DataMessageHandlerT handler)
{
    _defaultHandler = std::move(handler);
}

void DataSubscriberInternal::RegisterSpecificDataHandlerInternal(DataMessageHandlerT handler)
{
    _specificHandlers.push_back(handler);
}

void DataSubscriberInternal::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessageEvent& dataMessageEvent)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(dataMessageEvent);
}

void DataSubscriberInternal::ReceiveMessage(const DataMessageEvent& dataMessageEvent)
{
    bool anySpecificHandlerExecuted{false};
    if (!_specificHandlers.empty())
    {
        for (auto handler : _specificHandlers)
        {
            if (handler)
            {
                handler(_parent, dataMessageEvent);
                anySpecificHandlerExecuted = true;
            }
        }
    }

    if (_defaultHandler && !anySpecificHandlerExecuted)
    {
        _defaultHandler(_parent, dataMessageEvent);
    }

    if (!_defaultHandler && !anySpecificHandlerExecuted)
    {
        _participant->GetLogger()->Warn("DataSubscriber on topic " + _topic + " received data, but has no default or specific handler assigned");
    }
}

void DataSubscriberInternal::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace data
} // namespace sim
} // namespace ib
