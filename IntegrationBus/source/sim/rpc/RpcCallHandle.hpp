// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/rpc/IRpcCallHandle.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

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
    
    bool Valid() const override { return _callUUID.ab != 0 || _callUUID.cd != 0; }

    CallUUID _callUUID;
};

} // namespace rpc
} // namespace sim
} // namespace ib
