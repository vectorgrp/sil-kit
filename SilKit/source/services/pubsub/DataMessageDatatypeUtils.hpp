// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <unordered_map>

#include "silkit/services/pubsub/DataMessageDatatypes.hpp"
#include "silkit/util/HandlerId.hpp"

#include "Hash.hpp"
#include "WireDataMessages.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriberInternal;

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs);

bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs);

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
        uint64_t operator()(const SourceInfo& s) const { 
            auto hMediaType = SilKit::Util::Hash::Hash(s.mediaType);
            auto hLabels = SilKit::Util::Hash::Hash(s.labels);
            return SilKit::Util::Hash::HashCombine(hMediaType, hLabels);
        }
    };
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
