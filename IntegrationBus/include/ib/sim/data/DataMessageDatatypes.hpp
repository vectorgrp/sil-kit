// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <map>
#include <string>
#include <chrono>

namespace ib {
namespace sim {
//! The namespace for Data Message
namespace data {

class IDataSubscriber;
class IDataPublisher;

//! \brief An incoming DataMessage of a DataPublisher containing raw data and timestamp
struct DataMessageEvent
{
    //! Send timestamp of the event
    std::chrono::nanoseconds timestamp;
    //! Data field containing the payload
    std::vector<uint8_t> data;
};

//! \brief Callback type for new data reception callbacks
using DataMessageHandlerT =
    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent)>;

//! \brief Information about a newly discovered DataPublisher
struct NewDataPublisherEvent
{
    //! Reception timestamp of the event
    std::chrono::nanoseconds timestamp;
    //! The topic string of the discovered DataPublisher.
    std::string topic;
    //! The mediaType of the discovered DataPublisher.
    std::string mediaType;
    //! The labels of the discovered DataPublisher.
    std::map<std::string, std::string> labels;
};

//! \brief Callback type for new data publishers
using NewDataPublisherHandlerT =
    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const NewDataPublisherEvent& newDataPublisherEvent)>;

} // namespace data
} // namespace sim
} // namespace ib

