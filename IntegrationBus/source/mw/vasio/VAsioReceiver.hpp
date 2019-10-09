// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IbLink.hpp"

#include "VAsioMessageSubscriber.hpp"

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
class VAsioReceiver : public IVAsioReceiver
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

private:
    // ----------------------------------------
    // private members
    VAsioMsgSubscriber _subscriptionInfo;
    std::shared_ptr<IbLink<MsgT>> _link;
    logging::ILogger* _logger;
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

    _logger->Trace("Receiving {} Message from Endpoint Address ({}, {})", _subscriptionInfo.msgTypeName, endpoint.participant, endpoint.endpoint);
    _link->DistributeRemoteIbMessage(endpoint, msg);
}


template <>
class VAsioReceiver<logging::LogMsg> : public IVAsioReceiver
{
public:
    using MsgT = logging::LogMsg;
public:
    // ----------------------------------------
    // Constructors and Destructor
    inline VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<IbLink<MsgT>> link, logging::ILogger* /*logger*/);

public:
    // ----------------------------------------
    // Public interface methods
    inline auto GetDescriptor() const -> const VAsioMsgSubscriber& override;
    inline void ReceiveRawMsg(MessageBuffer&& buffer) override;

private:
    // ----------------------------------------
    // private members
    VAsioMsgSubscriber _subscriptionInfo;
    std::shared_ptr<IbLink<MsgT>> _link;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
VAsioReceiver<logging::LogMsg>::VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<IbLink<logging::LogMsg>> link, logging::ILogger* /*logger*/)
    : _subscriptionInfo{std::move(subscriberInfo)}
    , _link{link}
{
}

auto VAsioReceiver<logging::LogMsg>::GetDescriptor() const -> const VAsioMsgSubscriber&
{
    return _subscriptionInfo;
}

void VAsioReceiver<logging::LogMsg>::ReceiveRawMsg(MessageBuffer&& buffer)
{
    EndpointAddress endpoint;
    logging::LogMsg msg;
    buffer >> endpoint >> msg;

    _link->DistributeRemoteIbMessage(endpoint, msg);
}

} // mw
} // namespace ib
