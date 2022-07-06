// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include "internal_fwd.hpp"

namespace SilKit {
namespace Core {

// the silkit services type traits
template <class SilKitServiceT> struct SilKitServiceTraitUseAsyncRegistration { static constexpr bool UseAsyncRegistration() { return false; } };

// The final service traits
template <class SilKitServiceT> struct SilKitServiceTraits
    : SilKitServiceTraitUseAsyncRegistration<SilKitServiceT>
{
};

#define DefineSilKitServiceTrait_UseAsyncRegistration(Namespace, ServiceName) template<> struct SilKitServiceTraitUseAsyncRegistration<Namespace::ServiceName>{\
    static constexpr bool UseAsyncRegistration() { return true;}\
    }

// Services that are registered asynchronously (= not in main thread but in IO-Worker thread on an incoming SilKitMessage) 
DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::PubSub, DataSubscriberInternal);
DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::Rpc, RpcServerInternal);


} // namespace Core
} // namespace SilKit
