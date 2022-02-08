// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <map>

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
    std::string mediaType;
};

inline bool operator==(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs)
{
    return lhs.mediaType == rhs.mediaType;
}

//! \brief Callback type for new data reception callbacks
using DataHandlerT =
    std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data)>;

//! \brief Callback type for new data sources
using NewDataSourceHandlerT = std::function<void(ib::sim::data::IDataSubscriber* subscriber, const std::string& topic,
                                                 const ib::sim::data::DataExchangeFormat& dataExchangeFormat,
                                                 const std::map<std::string, std::string>& labels)>;


/*! \brief A data message
 *
 * Data messages run over an abstract channel, without timing effects and/or data type constraints
 */
struct DataMessage {
    std::vector<uint8_t> data;
};

} // namespace data
} // namespace sim
} // namespace ib

