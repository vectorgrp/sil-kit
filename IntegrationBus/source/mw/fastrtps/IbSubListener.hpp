// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbMessageReceiver.hpp"

#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"

#include <vector>

namespace ib {
namespace mw {

template<class IdlMessageT>
class IbSubListener : public eprosima::fastrtps::SubscriberListener
{
public:
    using IdlMessageType = IdlMessageT;
    using IbMessageType = to_ib_message_t<IdlMessageType>;
    using IbReceiver = IIbMessageReceiver<IbMessageType>;

public:
    IbSubListener() = default;

    void SetLogger(logging::ILogger* logger);
    void addReceiver(IbReceiver* receiver);
    void clearReceivers();
    
    inline void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;

private:
    logging::ILogger* _logger{nullptr};
    std::vector<IbReceiver*> _receivers;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
template<class IdlMessageT>
void IbSubListener<IdlMessageT>::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

template<class IdlMessageT>
void IbSubListener<IdlMessageT>::addReceiver(IbReceiver* receiver)
{
    if (std::find(_receivers.begin(), _receivers.end(), receiver) != _receivers.end())
        return;

    _receivers.push_back(receiver);
}
    
template<class IdlMessageT>
void IbSubListener<IdlMessageT>::clearReceivers()
{
    decltype(_receivers) emptyList;
    std::swap(_receivers, emptyList);
}
    
template<class IdlMessageT>
void IbSubListener<IdlMessageT>::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
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
        try
        {
            _logger->Trace("Receiving {} Message from Endpoint Address ({}, {})", TopicTrait<decltype(idlMsg)>::TypeName(), senderAddr.participant, senderAddr.endpoint);
            receiver->ReceiveIbMessage(senderAddr, msg);
        }
        catch (const std::exception& e)
        {
            std::cerr
                << "WARNING: Callback for "
                << sub->getAttributes().topic.topicDataType
                << "[\"" << sub->getAttributes().topic.topicName << "\"]"
                << " threw an exception: " << e.what() << "." << std::endl;
        }
        catch (...)
        {
            std::cerr
                << "WARNING: Callback for "
                << sub->getAttributes().topic.topicDataType
                << "[\"" << sub->getAttributes().topic.topicName << "\"]"
                << " threw an unknown exception." << std::endl;
        }
    }
}


template<>
class IbSubListener<logging::idl::LogMsg> : public eprosima::fastrtps::SubscriberListener
{
public:
    using IdlMessageType = logging::idl::LogMsg;
    using IbMessageType = to_ib_message_t<IdlMessageType>;
    using IbReceiver = IIbMessageReceiver<IbMessageType>;

public:
    IbSubListener() = default;

    inline void SetLogger(logging::ILogger* logger);
    inline void addReceiver(IbReceiver* receiver);
    inline void clearReceivers();

    inline void onNewDataMessage(eprosima::fastrtps::Subscriber* sub) override;

private:
    std::vector<IbReceiver*> _receivers;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
void IbSubListener<logging::idl::LogMsg>::SetLogger(logging::ILogger* /*logger*/)
{
}

void IbSubListener<logging::idl::LogMsg>::addReceiver(IbReceiver* receiver)
{
    if (std::find(_receivers.begin(), _receivers.end(), receiver) != _receivers.end())
        return;

    _receivers.push_back(receiver);
}

void IbSubListener<logging::idl::LogMsg>::clearReceivers()
{
    decltype(_receivers) emptyList;
    std::swap(_receivers, emptyList);
}

void IbSubListener<logging::idl::LogMsg>::onNewDataMessage(eprosima::fastrtps::Subscriber* sub)
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
        try
        {
            receiver->ReceiveIbMessage(senderAddr, msg);
        }
        catch (...)
        {
            std::cerr << "WARNING: Callback for remote logging threw an unknown exception." << std::endl;
        }
    }
}

} // mw
} // namespace ib
