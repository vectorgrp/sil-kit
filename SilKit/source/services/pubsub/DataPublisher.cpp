// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataPublisher.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataPublisher::DataPublisher(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                             const std::string& topic, const std::string& mediaType,
                             const std::map<std::string, std::string>& labels, const std::string& pubUUID)
    : _topic{topic}
    , _mediaType{mediaType}
    , _labels{labels}
    , _pubUUID{pubUUID}
    , _timeProvider{timeProvider}
    , _participant{participant}
{
}

void DataPublisher::Publish(std::vector<uint8_t> data)
{
    DataMessageEvent msg{_timeProvider->Now(), std::move(data)};
    _participant->SendMsg(this, std::move(msg));
}

void DataPublisher::Publish(const uint8_t* data, std::size_t size)
{
    Publish({data, data + size});
}

void DataPublisher::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
