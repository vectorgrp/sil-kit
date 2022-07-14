// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/rpc/IRpcCallHandle.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

#include "WireRpcMessages.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class CallHandleImpl : public IRpcCallHandle
{
  public:
    CallHandleImpl(CallUUID callUUID) : _callUUID{ callUUID.ab, callUUID.cd } {}

    // Operators for IRpcCallHandle
    bool operator==(const IRpcCallHandle& other) const override 
    { 
        auto otherCallUUID = static_cast<const CallHandleImpl&>(other)._callUUID; 
        return otherCallUUID.ab == _callUUID.ab && otherCallUUID.cd == _callUUID.cd;
    }
    bool operator!=(const IRpcCallHandle& other) const override
    {
        auto otherCallUUID = static_cast<const CallHandleImpl&>(other)._callUUID;
        return otherCallUUID.ab != _callUUID.ab || otherCallUUID.cd != _callUUID.cd;
    }
    
    CallUUID _callUUID;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
