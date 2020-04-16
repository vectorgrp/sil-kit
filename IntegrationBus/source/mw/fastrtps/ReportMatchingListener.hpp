// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"
#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/subscriber/SampleInfo.h"
#include "fastrtps/rtps/common/MatchingInfo.h"

class PubMatchedListener : public eprosima::fastrtps::PublisherListener
{
public:
    PubMatchedListener(::ib::mw::logging::ILogger* logger)
        : _logger{logger}
    {
    }

    void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            _logger->Debug("FastRTPS publisher [{}] {} matched",
                pub->getAttributes().topic.topicName, pub->getAttributes().topic.topicDataType);
        }
        else
        {
            _logger->Debug("FastRTPS publisher [{}] {} unmatched",
                pub->getAttributes().topic.topicName, pub->getAttributes().topic.topicDataType);
        }
    }

private:
    ::ib::mw::logging::ILogger* _logger;
};

class SubMatchedListener : public eprosima::fastrtps::SubscriberListener
{
public:
    SubMatchedListener(::ib::mw::logging::ILogger* logger)
        : _logger{logger}
    {
    }

    void onSubscriptionMatched(eprosima::fastrtps::Subscriber* sub, eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            _logger->Debug("FastRTPS subscriber [{}] {} matched",
                sub->getAttributes().topic.topicName, sub->getAttributes().topic.topicDataType);
        }
        else
        {
            _logger->Debug("FastRTPS subscriber [{}] {} unmatched",
                sub->getAttributes().topic.topicName, sub->getAttributes().topic.topicDataType);
        }
    }

private:
    ::ib::mw::logging::ILogger* _logger;
};
