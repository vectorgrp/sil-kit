// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

namespace {

static bool wildcardStringMatch(const std::string& s1, const std::string& s2)
{
    return s1 == "*" || s2 == "*" || s1 == s2;
}

} // anonymous namespace

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
