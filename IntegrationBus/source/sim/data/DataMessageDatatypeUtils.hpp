// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <set>
#include <memory>

#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "Hash.hpp"

namespace ib {
namespace sim {
namespace data {

class DataSubscriberInternal;

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs);

bool MatchMediaType(const std::string& subMediaType, const std::string& pubMediaType);

bool MatchLabels(const std::map<std::string, std::string>& innerSet, const std::map<std::string, std::string>& outerSet);

struct SpecificDataHandler
{
    uint64_t id;
    std::string mediaType;
    std::map<std::string, std::string> labels;
    DataMessageHandlerT specificDataHandler;
    std::set<DataSubscriberInternal*> registeredInternalSubscribers;
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
