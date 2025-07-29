// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataPublisher.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "WireDataMessages.hpp"
#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataPublisher::DataPublisher(Core::IParticipantInternal* participant,
                             Services::Orchestration::ITimeProvider* timeProvider,
                             const SilKit::Services::PubSub::PubSubSpec& dataSpec, const std::string& pubUUID,
                             const Config::DataPublisher& config)
    : _topic{dataSpec.Topic()}
    , _mediaType{dataSpec.MediaType()}
    , _labels{dataSpec.Labels()}
    , _pubUUID{pubUUID}
    , _timeProvider{timeProvider}
    , _participant{participant}
    , _config{config}
{
}

void DataPublisher::PublishInternal(Util::Span<const uint8_t> data)
{
    WireDataMessageEvent msg{_timeProvider->Now(), data};
    _tracer.Trace(SilKit::Services::TransmitDirection::TX, msg.timestamp, ToDataMessageEvent(msg));
    _participant->SendMsg(this, msg);
}

void DataPublisher::Publish(Util::Span<const uint8_t> data)
{
    if (Tracing::IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
    {
        return;
    }
    PublishInternal(data);
}

void DataPublisher::ReplayMessage(const SilKit::IReplayMessage* message)
{
    using namespace SilKit::Tracing;
    switch (message->GetDirection())
    {
    case SilKit::Services::TransmitDirection::TX:
        if (IsReplayEnabledFor(_config.replay, Config::Replay::Direction::Send))
        {
            auto&& msg = dynamic_cast<const WireDataMessageEvent&>(*message);
            PublishInternal(msg.data.AsSpan());
        }
        break;
    case SilKit::Services::TransmitDirection::RX:
        break;
    default:
        throw SilKitError("DataPublisher: replay message has undefined Direction");
        break;
    }
}

void DataPublisher::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
