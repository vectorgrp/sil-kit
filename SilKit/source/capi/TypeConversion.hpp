// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Types.h"

#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/rpc/RpcSpec.hpp"

#include <vector>
#include <string>

namespace {

inline void assign(SilKit::Services::PubSub::PubSubSpec& cppPubSubSpec, SilKit_DataSpec* pubSubSpec)
{
    cppPubSubSpec = {pubSubSpec->topic, pubSubSpec->mediaType};
    for (size_t i = 0; i < pubSubSpec->labelList.numLabels; i++)
    {
        cppPubSubSpec.AddLabel(pubSubSpec->labelList.labels[i].key, pubSubSpec->labelList.labels[i].value,
                               (SilKit::Services::MatchingLabel::Kind)pubSubSpec->labelList.labels[i].kind);
    }
}

inline void assign(SilKit::Services::Rpc::RpcSpec& cppRpcSpec, SilKit_RpcSpec* rpcSpec)
{
    cppRpcSpec = {rpcSpec->functionName, rpcSpec->mediaType};
    for (size_t i = 0; i < rpcSpec->labelList.numLabels; i++)
    {
        cppRpcSpec.AddLabel(rpcSpec->labelList.labels[i].key, rpcSpec->labelList.labels[i].value,
                            (SilKit::Services::MatchingLabel::Kind)rpcSpec->labelList.labels[i].kind);
    }
}

inline void assign(std::vector<std::string>& cppStrings, const SilKit_StringList* cStrings)
{
    if (cStrings)
    {
        for (uint32_t i = 0; i < cStrings->numStrings; i++)
        {
            cppStrings.push_back(cStrings->strings[i]);
        }
    }
}

} // anonymous namespace
