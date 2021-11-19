// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IbLink.hpp"

#include "VAsioDatatypes.hpp"

#include "MessageTracing.hpp"
#include "IServiceId.hpp"

namespace ib {
namespace mw {

class MessageBuffer;

class IVAsioReceiver
{
public:
    // ----------------------------------------
    // Public interface methods
    virtual ~IVAsioReceiver() = default;
    virtual auto GetDescriptor() const -> const VAsioMsgSubscriber& = 0;
    virtual void ReceiveRawMsg(MessageBuffer&& buffer) = 0;
};

template <class MsgT>
class VAsioReceiver
    : public IVAsioReceiver
    , public IServiceId
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<IbLink<MsgT>> link, logging::ILogger* logger);

public:
    // ----------------------------------------
    // Public interface methods
    auto GetDescriptor() const -> const VAsioMsgSubscriber& override;
    void ReceiveRawMsg(MessageBuffer&& buffer) override;
    void SetServiceId(const ServiceId& serviceId) override
    {
        _serviceId = serviceId;
    }
    auto GetServiceId() const -> const ServiceId& override
    {
        return _serviceId;
    }

private:
    // ----------------------------------------
    // private members
    VAsioMsgSubscriber _subscriptionInfo;
    std::shared_ptr<IbLink<MsgT>> _link;
    logging::ILogger* _logger;
    ServiceId _serviceId;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
VAsioReceiver<MsgT>::VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<IbLink<MsgT>> link, logging::ILogger* logger)
    : _subscriptionInfo{std::move(subscriberInfo)}
    , _link{link}
    , _logger{logger}
{
    _serviceId.linkName = _subscriptionInfo.linkName;
}

template <class MsgT>
auto VAsioReceiver<MsgT>::GetDescriptor() const -> const VAsioMsgSubscriber&
{
    return _subscriptionInfo;
}

template <class MsgT>
void VAsioReceiver<MsgT>::ReceiveRawMsg(MessageBuffer&& buffer)
{
    EndpointAddress endpoint;
    MsgT msg;
    buffer >> endpoint >> msg;

    TraceRx(_logger, endpoint, msg);
    _serviceId.legacyEpa = endpoint;
    _link->DistributeRemoteIbMessage(this, msg);
}


} // mw
} // namespace ib
