/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "DataPublisher.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "WireDataMessages.hpp"

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

void DataPublisher::Publish(Util::Span<const uint8_t> data)
{
    WireDataMessageEvent msg{_timeProvider->Now(), data};
    _participant->SendMsg(this, msg);
}

void DataPublisher::SetTimeProvider(Services::Orchestration::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
