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

#include "traits/SilKitServices_fwd.hpp"

namespace SilKit {


//////////////////////////////////////////////////////////////////////
//Trait: associate a configuration type to the service type
//////////////////////////////////////////////////////////////////////
template <class SilKitServiceT>
struct SilKitServiceTraitConfigType {
    // not defined by default: using type = SomeType
    using type = void;
};

template<typename ServiceT>
using SilKitServiceTraitConfigType_t = typename SilKitServiceTraitConfigType<ServiceT>::type;

#define DefineSilKitServiceTrait_ConfigType(SERVICE_NAME, CONFIG_TYPE) \
    template<> \
    struct SilKitServiceTraitConfigType<SERVICE_NAME>{\
        using type = CONFIG_TYPE; \
    }

DefineSilKitServiceTrait_ConfigType(SilKit::Services::Can::CanController, SilKit::Config::CanController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Ethernet::EthController, SilKit::Config::EthernetController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Lin::LinController, SilKit::Config::LinController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Flexray::FlexrayController, SilKit::Config::FlexrayController);

DefineSilKitServiceTrait_ConfigType(SilKit::Services::PubSub::DataPublisher, SilKit::Config::DataPublisher);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::PubSub::DataSubscriber, SilKit::Config::DataSubscriber);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::PubSub::DataSubscriberInternal, SilKit::Config::DataSubscriber);

DefineSilKitServiceTrait_ConfigType(SilKit::Services::Rpc::RpcClient, SilKit::Config::RpcClient);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Rpc::RpcServer, SilKit::Config::RpcServer);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Rpc::RpcServerInternal, SilKit::Config::RpcServer);

DefineSilKitServiceTrait_ConfigType(SilKit::Services::Logging::LogMsgReceiver, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Logging::LogMsgSender, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::TimeSyncService, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::LifecycleService, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::SystemMonitor, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::SystemController, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Core::Discovery::ServiceDiscovery, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Core::RequestReply::RequestReplyService, SilKit::Config::InternalController);


//////////////////////////////////////////////////////////////////////
//Trait: associate a ServiceType attribute with a service type
//////////////////////////////////////////////////////////////////////

template <class SilKitServiceT>
struct SilKitServiceTraitServiceType {
    constexpr static auto GetServiceType() ->  Core::ServiceType {
        return Core::ServiceType::Controller; // Most services are actual controllers
    }
};

#define DefineSilKitServiceTrait_ServiceType(SERVICE_NAME, SERVICE_TYPE_VALUE) \
    template<> \
    struct SilKitServiceTraitServiceType<SERVICE_NAME>{\
        constexpr static auto GetServiceType() -> Core::ServiceType \
        { \
            return SERVICE_TYPE_VALUE; \
        } \
    }

DefineSilKitServiceTrait_ServiceType(SilKit::Core::Discovery::ServiceDiscovery, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::SystemController, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::SystemMonitor, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::TimeSyncService, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::LifecycleService, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Logging::LogMsgReceiver, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Logging::LogMsgSender, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Core::RequestReply::RequestReplyService, SilKit::Core::ServiceType::InternalController);
} // namespace SilKit
