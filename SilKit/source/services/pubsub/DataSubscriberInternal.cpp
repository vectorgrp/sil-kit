// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSubscriberInternal.hpp"

#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataSubscriberInternal::DataSubscriberInternal(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider, 
                                               const std::string& topic, const std::string& mediaType,
                                               const std::map<std::string, std::string>& labels,
                                               DataMessageHandlerT defaultHandler, IDataSubscriber* parent)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _defaultHandler{std::move(defaultHandler)}
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

auto DataSubscriberInternal::AddExplicitDataMessageHandler(DataMessageHandlerT handler) -> HandlerId
{
    return _explicitDataMessageHandlers.Add(std::move(handler));
}

void DataSubscriberInternal::RemoveExplicitDataMessageHandler(HandlerId handlerId)
{
    _explicitDataMessageHandlers.Remove(handlerId);
}

void DataSubscriberInternal::ReceiveSilKitMessage(const Core::IServiceEndpoint* from, const DataMessageEvent& dataMessageEvent)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;

    ReceiveMessage(dataMessageEvent);
}

void DataSubscriberInternal::ReceiveMessage(const DataMessageEvent& dataMessageEvent)
{
    const auto anySpecificHandlerExecuted = _explicitDataMessageHandlers.InvokeAll(_parent, dataMessageEvent);

    if (_defaultHandler && !anySpecificHandlerExecuted)
    {
        _defaultHandler(_parent, dataMessageEvent);
    }

    if (!_defaultHandler && !anySpecificHandlerExecuted)
    {
        _participant->GetLogger()->Warn("DataSubscriber on topic " + _topic + " received data, but has no default or specific handler assigned");
    }
}

void DataSubscriberInternal::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}


} // namespace PubSub
} // namespace Services
} // namespace SilKit
