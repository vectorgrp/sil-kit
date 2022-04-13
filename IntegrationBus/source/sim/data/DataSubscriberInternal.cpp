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
                                               DataHandlerT defaultHandler, IDataSubscriber* parent)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _defaultHandler{defaultHandler}
    , _parent{parent}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void DataSubscriberInternal::SetDefaultReceiveMessageHandler(DataHandlerT handler)
{
    _defaultHandler = std::move(handler);
}

void DataSubscriberInternal::RegisterSpecificDataHandlerInternal(DataHandlerT handler)
{
    _specificHandlers.push_back(handler);
}

void DataSubscriberInternal::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessage& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(msg.data);
}

void DataSubscriberInternal::ReceiveMessage(const std::vector<uint8_t>& data)
{
    bool anySpecificHandlerExecuted{false};
    if (!_specificHandlers.empty())
    {
        for (auto handler : _specificHandlers)
        {
            if (handler)
            {
                handler(_parent, data);
                anySpecificHandlerExecuted = true;
            }
        }
    }

    if (_defaultHandler && !anySpecificHandlerExecuted)
    {
        _defaultHandler(_parent, data);
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
