// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <unordered_map>

#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/util/HandlerId.hpp"

#include "Hash.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriberInternal;

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs);

bool MatchMediaType(const std::string& subMediaType, const std::string& pubMediaType);

bool MatchLabels(const std::map<std::string, std::string>& innerSet, const std::map<std::string, std::string>& outerSet);

struct ExplicitDataMessageHandlerInfo
{
    HandlerId id;
    std::string mediaType;
    std::map<std::string, std::string> labels;
    DataMessageHandlerT explicitDataMessageHandler;
    std::unordered_map<DataSubscriberInternal*, HandlerId> registeredInternalSubscribers;
};


struct SourceInfo
{
    std::string mediaType;
    std::map<std::string, std::string> labels;

    bool operator==(const SourceInfo& other) const
    {
        return other.mediaType == mediaType && std::equal(other.labels.begin(), other.labels.end(), labels.begin());
    }
    struct HashFunction
    {
        size_t operator()(const SourceInfo& s) const { 
            auto hMediaType = ib::util::hash::Hash(s.mediaType);
            auto hLabels = ib::util::hash::Hash(s.labels);
            return ib::util::hash::HashCombine(hMediaType, hLabels);
        }
    };
};

} // namespace data
} // namespace sim
} // namespace ib
