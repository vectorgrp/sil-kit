// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include <vector>

namespace ib {
namespace mw {

template<class IdlMessageT>
class IbSubListenerNg : public eprosima::fastrtps::SubscriberListener
{
public:
    using IdlMessageType = IdlMessageT;
    using IbMessageType = to_ib_message_t<IdlMessageType>;
    using IbReceiver = IIbMessageReceiver<IbMessageType>;

public:
    IbSubListenerNg() = default;

    void addReceiver(IbReceiver* receiver);
    void clearReceivers();
    
    void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;

private:
    std::vector<IbReceiver*> _receivers;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IdlMessageT>
void IbSubListenerNg<IdlMessageT>::addReceiver(IbReceiver* receiver)
{
    _receivers.push_back(receiver);
}
    
template<class IdlMessageT>
void IbSubListenerNg<IdlMessageT>::clearReceivers()
{
    decltype(_receivers) emptyList;
    std::swap(_receivers, emptyList);
}
    
template<class IdlMessageT>
void IbSubListenerNg<IdlMessageT>::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
{
    IdlMessageType idlMsg;
	eprosima::fastrtps::SampleInfo_t info;

    if (!sub->takeNextData(&idlMsg, &info))
        return;

    if (info.sampleKind != eprosima::fastrtps::rtps::ALIVE)
        return;

    auto senderAddr = from_idl(idlMsg.senderAddr());
    auto msg = from_idl(std::move(idlMsg));

    for (auto&& receiver : _receivers)
    {
        receiver->ReceiveIbMessage(senderAddr, msg);
    }
}


} // mw
} // namespace ib
