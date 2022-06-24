// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ITimeConsumer.hpp"

#include "IIbToDataSubscriberInternal.hpp"
#include "IParticipantInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"
#include "SynchronizedHandlers.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriberInternal
    : public IIbToDataSubscriberInternal
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    DataSubscriberInternal(mw::IParticipantInternal* participant, mw::sync::ITimeProvider* timeProvider,
                           const std::string& topic, const std::string& mediaType,
                           const std::map<std::string, std::string>& labels, DataMessageHandlerT defaultHandler,
                           IDataSubscriber* parent);

    void SetDefaultDataMessageHandler(DataMessageHandlerT handler);

    auto AddExplicitDataMessageHandler(DataMessageHandlerT handler) -> HandlerId;

    void RemoveExplicitDataMessageHandler(HandlerId handlerId);

    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const DataMessageEvent& dataMessageEvent) override;

    void ReceiveMessage(const DataMessageEvent& dataMessageEvent);

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    std::string GetMediaType() { return _mediaType; };
    std::map <std::string, std::string> GetLabels() { return _labels; };

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    std::string _topic;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    DataMessageHandlerT _defaultHandler;
    util::SynchronizedHandlers<DataMessageHandlerT> _explicitDataMessageHandlers;

    IDataSubscriber* _parent{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    mw::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataSubscriberInternal::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataSubscriberInternal::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace data
} // namespace sim
} // namespace ib
