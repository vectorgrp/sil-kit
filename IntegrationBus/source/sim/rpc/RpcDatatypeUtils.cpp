// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "RpcDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace rpc {

bool MatchMediaType(const std::string& clientMediaType, const std::string& serverMediaType)
{
    return clientMediaType == "" || clientMediaType == serverMediaType;
}

bool MatchLabels(const std::map<std::string, std::string>& clientLabels,
                 const std::map<std::string, std::string>& serverLabels)
{
    if (clientLabels.size() == 0)
        return true; // clientLabels empty -> match

    if (clientLabels.size() > serverLabels.size())
        return false; // clientLabels more labels than outer set -> no match

    for (auto&& kv : clientLabels)
    {
        auto it = serverLabels.find(kv.first);
        if (it == serverLabels.end() || // Key not found -> no match
            (kv.second != "" && kv.second != (*it).second)) // Value does not match (and no wildcard given) -> no match
        {
            return false;
        }
    }
    return true; // All of clientLabels are there -> match
}

} // namespace rpc
} // namespace sim
} // namespace ib
