// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
