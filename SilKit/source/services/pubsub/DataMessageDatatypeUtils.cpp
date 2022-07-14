// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataMessageDatatypeUtils.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs)
{
    return Util::ItemsAreEqual(lhs.data, rhs.data);
}

bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs)
{
    return ToDataMessageEvent(lhs) == ToDataMessageEvent(rhs);
}

bool MatchMediaType(const std::string& subMediaType, const std::string& pubMediaType)
{
    return subMediaType == "" || subMediaType == pubMediaType;
}

bool MatchLabels(const std::map<std::string, std::string>& subscriberLabels, const std::map<std::string, std::string>& publisherLabels)
{
    if (subscriberLabels.size() == 0)
        return true; // subscriberLabels empty -> match

    if (subscriberLabels.size() > publisherLabels.size())
        return false; // subscriberLabels more labels than outer set -> no match

    for (auto&& kv : subscriberLabels)
    {
        auto it = publisherLabels.find(kv.first);
        if (it == publisherLabels.end() || // Key not found -> no match
            (kv.second != "" && kv.second != (*it).second)) // Value does not match (and no wildcard given) -> no match
        {
            return false;
        }
    }
    return true; // All of subscriberLabels is there -> match
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
