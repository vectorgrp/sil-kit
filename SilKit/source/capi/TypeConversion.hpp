/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
