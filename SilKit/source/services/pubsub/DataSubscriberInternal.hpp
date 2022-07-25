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

#pragma once

#include "ITimeConsumer.hpp"

#include "IMsgForDataSubscriberInternal.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriberInternal
    : public IMsgForDataSubscriberInternal
    , public Services::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    DataSubscriberInternal(Core::IParticipantInternal* participant, Services::Orchestration::ITimeProvider* timeProvider,
                           const std::string& topic, const std::string& mediaType,
                           const std::map<std::string, std::string>& labels, DataMessageHandlerT defaultHandler,
                           IDataSubscriber* parent);

    void SetDefaultDataMessageHandler(DataMessageHandlerT handler);

    auto AddExplicitDataMessageHandler(DataMessageHandlerT handler) -> HandlerId;

    void RemoveExplicitDataMessageHandler(HandlerId handlerId);

    //! \brief Accepts messages originating from SIL Kit communications.
    void ReceiveMsg(const Core::IServiceEndpoint* from, const WireDataMessageEvent& dataMessageEvent) override;

    void ReceiveMessage(const WireDataMessageEvent& dataMessageEvent);

    //SilKit::Services::Orchestration::ITimeConsumer
    void SetTimeProvider(Services::Orchestration::ITimeProvider* provider) override;

    std::string GetMediaType() { return _mediaType; };
    std::map <std::string, std::string> GetLabels() { return _labels; };

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    std::string _topic;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    DataMessageHandlerT _defaultHandler;
    Util::SynchronizedHandlers<DataMessageHandlerT> _explicitDataMessageHandlers;

    IDataSubscriber* _parent{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriberInternal::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriberInternal::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
