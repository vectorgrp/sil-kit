// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "internal_fwd.hpp"

namespace ib {
namespace mw {

// the ib services type traits
template <class IbServiceT> struct IbServiceTraitUseAsyncRegistration { static constexpr bool UseAsyncRegistration() { return false; } };

// The final service traits
template <class IbServiceT> struct IbServiceTraits
    : IbServiceTraitUseAsyncRegistration<IbServiceT>
{
};

#define DefineIbServiceTrait_UseAsyncRegistration(Namespace, ServiceName) template<> struct IbServiceTraitUseAsyncRegistration<Namespace::ServiceName>{\
    static constexpr bool UseAsyncRegistration() { return true;}\
    }

// Services that are registered asynchronously (= not in main thread but in IO-Worker thread on an incoming IbMessage) 
DefineIbServiceTrait_UseAsyncRegistration(ib::sim::data, DataSubscriberInternal);
DefineIbServiceTrait_UseAsyncRegistration(ib::sim::rpc, RpcServerInternal);


} // mw
} // namespace ib
