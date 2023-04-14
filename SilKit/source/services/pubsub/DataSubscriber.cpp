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

#include "DataSubscriber.hpp"
#include "IServiceDiscovery.hpp"
#include "YamlParser.hpp"
#include "LabelMatching.hpp"

#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

DataSubscriber::DataSubscriber(Core::IParticipantInternal* participant, Config::DataSubscriber config,
                               Services::Orchestration::ITimeProvider* timeProvider,
                               const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                               DataMessageHandler defaultDataHandler)
    : _topic{dataSpec.Topic()}
    , _mediaType{dataSpec.MediaType()}
    , _labels{dataSpec.Labels()}
    , _defaultDataHandler{WrapTracingCallback(defaultDataHandler)}
    , _timeProvider{timeProvider}
    , _participant{ participant }
    , _config{std::move(config)}
{
}

void DataSubscriber::RegisterServiceDiscovery()
{
    auto matchHandler = [this](SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                               const SilKit::Core::ServiceDescriptor& serviceDescriptor) {
        auto getVal = [serviceDescriptor](const std::string& key) {
            std::string tmp;
            if (!serviceDescriptor.GetSupplementalDataItem(key, tmp))
            {
                throw SilKitError{"Unknown key in supplementalData"};
            }
            return tmp;
        };

        const auto pubUUID = getVal(Core::Discovery::supplKeyDataPublisherPubUUID);

        // Early abort creation if Publisher is already connected
        if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
            && _internalSubscribers.count(pubUUID) > 0)
        {
            return;
        }

        const auto topic = getVal(Core::Discovery::supplKeyDataPublisherTopic);
        if (topic == _topic)
        {
            const std::string pubMediaType{getVal(Core::Discovery::supplKeyDataPublisherMediaType)};
            if (MatchMediaType(_mediaType, pubMediaType))
            {
                const std::string labelsStr = getVal(Core::Discovery::supplKeyDataPublisherPubLabels);
                const std::vector<SilKit::Services::MatchingLabel> publisherLabels =
                    SilKit::Config::Deserialize<std::vector<SilKit::Services::MatchingLabel>>(labelsStr);
                if (Util::MatchLabels(_labels, publisherLabels))
                {
                    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);

                    if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                    {
                        AddInternalSubscriber(pubUUID, pubMediaType, publisherLabels);
                    }
                    else if (discoveryType == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved)
                    {
                        RemoveInternalSubscriber(pubUUID);
                    }
                }
            }
        }
    };

    _participant->GetServiceDiscovery()->RegisterSpecificServiceDiscoveryHandler(
        matchHandler, Core::Discovery::controllerTypeDataPublisher, _topic, _labels);
}


void DataSubscriber::SetDataMessageHandler(DataMessageHandler callback)
{
    std::unique_lock<decltype(_internalSubscribersMx)> lock(_internalSubscribersMx);
    auto tracingCallback = WrapTracingCallback(std::move(callback));
    _defaultDataHandler = tracingCallback;
    for (auto internalSubscriber : _internalSubscribers)
    {
        internalSubscriber.second->SetDataMessageHandler(tracingCallback);
    }
}

void DataSubscriber::AddInternalSubscriber(const std::string& pubUUID, const std::string& joinedMediaType,
                                           const std::vector<SilKit::Services::MatchingLabel>& publisherLabels)
{
    auto internalSubscriber = dynamic_cast<DataSubscriberInternal*>(_participant->CreateDataSubscriberInternal(
        _topic, pubUUID, joinedMediaType, publisherLabels, _defaultDataHandler, this));
    
    _internalSubscribers.emplace(pubUUID, internalSubscriber);
}

void DataSubscriber::RemoveInternalSubscriber(const std::string& pubUUID)
{
    auto internalSubscriber = _internalSubscribers.find(pubUUID);
    if (internalSubscriber != _internalSubscribers.end())
    {
        _participant->GetServiceDiscovery()->NotifyServiceRemoved(internalSubscriber->second->GetServiceDescriptor());
        _internalSubscribers.erase(pubUUID);
    }
}

 auto DataSubscriber::WrapTracingCallback(DataMessageHandler callback) ->
    SilKit::Services::PubSub::DataMessageHandler
{
    auto tracingCallback = [this, callback=std::move(callback)](auto&& service, auto&& message) {
        _tracer.Trace(TransmitDirection::RX, _timeProvider->Now(), message);
        if (callback)
        {
            callback(service, message);
        }
    };
    return tracingCallback;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
