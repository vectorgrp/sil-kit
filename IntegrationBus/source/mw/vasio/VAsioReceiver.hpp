// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IbLink.hpp"

#include "VAsioDatatypes.hpp"

#include "MessageTracing.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {


struct RemoteServiceEndpoint : IIbServiceEndpoint
{
    void SetServiceId(const ib::mw::ServiceId&) override 
    { 
        throw std::logic_error("This method is not supposed to be used in this struct."); 
    }

    auto GetServiceId() const -> const ServiceId & override
    { 
        return _id; 
    }

    RemoteServiceEndpoint(IVAsioPeer* remoteParticipant, const IIbServiceEndpoint* receiver)
    {
        _id.participantName = remoteParticipant->GetUri().participantName;
        const auto& receiverServiceId = receiver->GetServiceId();
        _id.serviceName = receiverServiceId.serviceName;
        _id.linkName = receiverServiceId.linkName;
        _id.legacyEpa = receiverServiceId.legacyEpa;
        _id.isLinkSimulated = receiverServiceId.isLinkSimulated;
        _id.type = receiverServiceId.type;
    }

private:
    ServiceId _id;
};

class MessageBuffer;

class IVAsioReceiver
{
public:
    // ----------------------------------------
    // Public interface methods
    virtual ~IVAsioReceiver() = default;
    virtual auto GetDescriptor() const -> const VAsioMsgSubscriber& = 0;
    virtual void ReceiveRawMsg(IVAsioPeer* from, MessageBuffer&& buffer) = 0;
};

template <class MsgT>
class VAsioReceiver
    : public IVAsioReceiver
    , public IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<IbLink<MsgT>> link, logging::ILogger* logger);

public:
    // ----------------------------------------
    // Public interface methods
    auto GetDescriptor() const -> const VAsioMsgSubscriber& override;
    void ReceiveRawMsg(IVAsioPeer* from, MessageBuffer&& buffer) override;
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
void VAsioReceiver<MsgT>::ReceiveRawMsg(IVAsioPeer* from, MessageBuffer&& buffer)
{
    EndpointAddress endpoint;
    MsgT msg;
    buffer >> endpoint >> msg;

    TraceRx(_logger, this, msg);
    _serviceId.legacyEpa = endpoint;

    auto* fromService = dynamic_cast<IIbServiceEndpoint*>(from);
    ServiceId tmpService(fromService->GetServiceId());
    tmpService.legacyEpa = endpoint;

    //// TODO set data from peer?
    //_link->DistributeRemoteIbMessage(fromService, msg);


    fromService->SetServiceId(tmpService);

    auto remoteId = RemoteServiceEndpoint(from, this); 
    _link->DistributeRemoteIbMessage(&remoteId, msg);

}


} // mw
} // namespace ib
