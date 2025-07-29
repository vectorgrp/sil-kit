// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataSubscriberInternal.hpp"
#include "DataSubscriber.hpp"

#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataSubscriberInternal::DataSubscriberInternal(Core::IParticipantInternal* participant,
                                               Services::Orchestration::ITimeProvider* timeProvider,
                                               const std::string& topic, const std::string& mediaType,
                                               const std::vector<SilKit::Services::MatchingLabel>& labels,
                                               DataMessageHandler defaultHandler, IDataSubscriber* parent)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _defaultHandler{std::move(defaultHandler)}
    , _parent{parent}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
    (void)_participant;

    if (_parent)
    {
        _replayConfig = dynamic_cast<DataSubscriber&>(*_parent).GetConfig().replay;
    }
}

void DataSubscriberInternal::SetDataMessageHandler(DataMessageHandler handler)
{
    _defaultHandler = std::move(handler);
}

void DataSubscriberInternal::ReceiveMsg(const IServiceEndpoint* /*from*/, const WireDataMessageEvent& dataMessageEvent)
{
    if (Tracing::IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Receive))
    {
        return;
    }

    ReceiveInternal(dataMessageEvent);
}

void DataSubscriberInternal::ReceiveInternal(const WireDataMessageEvent& dataMessageEvent)
{
    if (_defaultHandler)
    {
        _defaultHandler(_parent, ToDataMessageEvent(dataMessageEvent));
    }

    if (!_defaultHandler)
    {
        _participant->GetLogger()->Warn("DataSubscriber on topic " + _topic
                                        + " received data, but has no default handler assigned");
    }
}

void DataSubscriberInternal::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

void DataSubscriberInternal::ReplayMessage(const IReplayMessage* message)
{
    using namespace SilKit::Tracing;
    switch (message->GetDirection())
    {
    case SilKit::Services::TransmitDirection::RX:
        if (IsReplayEnabledFor(_replayConfig, Config::Replay::Direction::Receive))
        {
            auto&& msg = dynamic_cast<const Services::PubSub::WireDataMessageEvent&>(*message);
            ReceiveInternal(msg);
        }
        break;
    case SilKit::Services::TransmitDirection::TX:
        //Ignore transmit messages
        break;
    default:
        throw SilKitError("DataSubscriber: replay message has undefined direction");
        break;
    }
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
