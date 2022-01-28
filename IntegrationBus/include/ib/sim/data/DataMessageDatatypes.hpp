// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace ib {
namespace sim {
//! The namespace for Data Message
namespace data {

class IDataSubscriber;
class IDataPublisher;

/*! \brief Serialization details.
 *
 * Specification of the data format used by individual DataPublishers and DataSubscribers. Single asterisk for wildcard
 * fields
 */
struct DataExchangeFormat {
    std::string mimeType;
};

inline bool operator==(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs)
{
    return lhs.mimeType == rhs.mimeType;
}

//! \brief Callback type for new data reception callbacks
using CallbackExchangeFormatT = std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data, const ib::sim::data::DataExchangeFormat& dataExchangeFormat)>;

/*! \brief A data message
 *
 * Data messages run over an abstract channel, without timing effects and/or data type constraints
 */
struct DataMessage {
    std::vector<uint8_t> data;
};

struct PublisherAnnouncement
{
    std::string        topic;
    std::string        pubUUID;
    DataExchangeFormat pubDataExchangeFormat;
};

} // namespace data
} // namespace sim
} // namespace ib

