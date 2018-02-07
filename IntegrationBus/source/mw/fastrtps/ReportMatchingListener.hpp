// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/publisher/Publisher.h"
#include "fastrtps/subscriber/SubscriberListener.h"
#include "fastrtps/subscriber/Subscriber.h"

class PubMatchedListener : public eprosima::fastrtps::PublisherListener
{
public:
    PubMatchedListener() = default;

    void onPublicationMatched(eprosima::fastrtps::Publisher* pub, eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            _numMatched++;
            std::cout << "Publisher matched: "
                << pub->getAttributes().topic.topicDataType
                << "[\"" << pub->getAttributes().topic.topicName << "\"]"
                << std::endl;
        }
        else
        {
            _numMatched--;
            std::cout << "Publisher unmatched: "
                << pub->getAttributes().topic.topicDataType
                << "[\"" << pub->getAttributes().topic.topicName << "\"]"
                << std::endl;
        }
    }

    auto numMatched() const { return _numMatched; }

private:
    int _numMatched = 0;
};

class SubMatchedListener : public eprosima::fastrtps::SubscriberListener
{
public:
    SubMatchedListener() = default;

    void onSubscriptionMatched(eprosima::fastrtps::Subscriber* /*pub*/, eprosima::fastrtps::rtps::MatchingInfo& info) override
    {
        if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
        {
            _numMatched++;
            std::cout << "Subscriber matched" << std::endl;
        }
        else
        {
            _numMatched--;
            std::cout << "Subscriber unmatched" << std::endl;
        }
    }

    auto numMatched() const { return _numMatched; }

private:
    int _numMatched = 0;
};
