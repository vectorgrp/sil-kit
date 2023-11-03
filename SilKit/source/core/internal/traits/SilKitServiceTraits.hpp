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
#include "internal_fwd.hpp"

namespace SilKit {
namespace Core {

// the silkit services type traits
template <class SilKitServiceT>
struct SilKitServiceTraitUseAsyncRegistration {
    static constexpr bool UseAsyncRegistration() {
        return false;
    }
};

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

DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::Can, IMsgForCanSimulator);
DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::Ethernet, IMsgForEthSimulator);
DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::Flexray, IMsgForFlexraySimulator);
DefineSilKitServiceTrait_UseAsyncRegistration(SilKit::Services::Lin, IMsgForLinSimulator);


} // namespace Core
} // namespace SilKit
