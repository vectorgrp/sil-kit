// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

bool operator==(const DataMessage& lhs, const DataMessage& rhs)
{
    return lhs.data == rhs.data;
}

bool operator==(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs)
{
    return lhs.mimeType == rhs.mimeType;
}

static bool wildcardStringMatch(const std::string& s1, const std::string& s2)
{
    return s1 == "*" || s2 == "*" || s1 == s2;
}

bool Match(const DataExchangeFormat& lhs, const DataExchangeFormat& rhs)
{
    return wildcardStringMatch(lhs.mimeType, rhs.mimeType);
}

DataExchangeFormat Join(const DataExchangeFormat& defA, const DataExchangeFormat& defB)
{
    ib::sim::data::DataExchangeFormat defJoined;
    defJoined.mimeType = defA.mimeType == "*" ? defB.mimeType : defA.mimeType;
    return defJoined;
}

} // namespace data
} // namespace sim
} // namespace ib
