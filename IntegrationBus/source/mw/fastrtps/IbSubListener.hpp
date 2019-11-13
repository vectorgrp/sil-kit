// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"

#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/rtps/common/MatchingInfo.h"

#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"

#include <vector>
#include <map>

#include "MessageTracing.hpp"

namespace ib {
namespace mw {

template<class IdlMessageT>
class IbSubListenerBase
{
public:
    using IdlMessageType = IdlMessageT;
    using IbMessageType = to_ib_message_t<IdlMessageType>;
    using IbReceiver = IIbMessageReceiver<IbMessageType>;

public:
    inline void SetLogger(logging::ILogger* logger);
    inline void addReceiver(IbReceiver* receiver);
    inline void clearReceivers();

protected:
    inline void dispatchToReceivers(
        eprosima::fastrtps::Subscriber* sub,
        EndpointAddress senderAddr,
        const IbMessageType& msg
    );

protected:
    logging::ILogger* _logger{nullptr};
    std::vector<IbReceiver*> _receivers;
};

template<class IdlMessageT>
void IbSubListenerBase<IdlMessageT>::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

template<class IdlMessageT>
void IbSubListenerBase<IdlMessageT>::addReceiver(IbReceiver* receiver)
{
    if (std::find(_receivers.begin(), _receivers.end(), receiver) != _receivers.end())
        return;

    _receivers.push_back(receiver);
}

template<class IdlMessageT>
void IbSubListenerBase<IdlMessageT>::clearReceivers()
{
    decltype(_receivers) emptyList;
    std::swap(_receivers, emptyList);
}

template<class IdlMessageT>
void IbSubListenerBase<IdlMessageT>::dispatchToReceivers(
    eprosima::fastrtps::Subscriber* sub,
    EndpointAddress senderAddr,
    const IbMessageType& msg
)
{
    TraceRx(_logger, senderAddr, msg);

    for (auto&& receiver : _receivers)
    {
        try
        {
            receiver->ReceiveIbMessage(senderAddr, msg);
        }
        catch (const std::exception& e)
        {
            _logger->Warn("Callback for {}[\"{}\"] threw an exception: ",
                sub->getAttributes().topic.topicDataType,
                sub->getAttributes().topic.topicName,
                e.what());
        }
        catch (...)
        {
            _logger->Warn("Callback for {}[\"{}\"] threw an unknown exception",
                sub->getAttributes().topic.topicDataType,
                sub->getAttributes().topic.topicName);
        }
    }
}

// ================================================================================
template<class IdlMessageT>
class IbSubListener
    : public eprosima::fastrtps::SubscriberListener
    , public IbSubListenerBase<IdlMessageT>
{
public:
    IbSubListener() = default;

    inline void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;
};

template<class IdlMessageT>
void IbSubListener<IdlMessageT>::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
{
    typename IbSubListenerBase<IdlMessageT>::IdlMessageType idlMsg;
    eprosima::fastrtps::SampleInfo_t info;

    if (!sub->takeNextData(&idlMsg, &info))
        return;

    if (info.sampleKind != eprosima::fastrtps::rtps::ALIVE)
        return;

    auto senderAddr = from_idl(idlMsg.senderAddr());
    auto msg = from_idl(std::move(idlMsg));
    IbSubListenerBase<IdlMessageT>::dispatchToReceivers(sub, senderAddr, msg);
}

// ================================================================================
template<>
class IbSubListener<sync::idl::ParticipantStatus>
    : public eprosima::fastrtps::SubscriberListener
    , public IbSubListenerBase<sync::idl::ParticipantStatus>
{
public:
    IbSubListener() = default;

    inline void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;
    inline void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info) override;

private:
    struct ParticipantStatusWithSenderAddr {
        EndpointAddress senderAddr;
        sync::ParticipantStatus msg;
    };

    std::map<eprosima::fastrtps::rtps::GUID_t, ParticipantStatusWithSenderAddr> _lastParticipantStatus;
};

void IbSubListener<sync::idl::ParticipantStatus>::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
{
    sync::idl::ParticipantStatus idlMsg;
    eprosima::fastrtps::SampleInfo_t info;

    if (!sub->takeNextData(&idlMsg, &info))
        return;

    if (info.sampleKind != eprosima::fastrtps::rtps::ALIVE)
        return;

    auto senderAddr = from_idl(idlMsg.senderAddr());
    auto msg = from_idl(std::move(idlMsg));

    _lastParticipantStatus[sub->getGuid()] = { senderAddr, msg };

    IbSubListenerBase<sync::idl::ParticipantStatus>::dispatchToReceivers(sub, senderAddr, msg);
}

void IbSubListener<sync::idl::ParticipantStatus>::onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == eprosima::fastrtps::rtps::MatchingStatus::REMOVED_MATCHING)
    {
        auto iter = _lastParticipantStatus.find(sub->getGuid());
        if (iter != _lastParticipantStatus.end())
        {
            auto& msgWithAddr = iter->second;
            auto senderAddr = msgWithAddr.senderAddr;

            auto msg = msgWithAddr.msg;
            msg.state = sync::ParticipantState::Error;
            msg.enterReason = std::string{"Shutdown"};
            msg.enterTime = std::chrono::system_clock::now();
            msg.refreshTime = std::chrono::system_clock::now();

            IbSubListenerBase<sync::idl::ParticipantStatus>::dispatchToReceivers(sub, senderAddr, msg);
        }
    }
}


} // mw
} // namespace ib
