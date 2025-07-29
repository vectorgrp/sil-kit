// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "traits/SilKitServices_fwd.hpp"

namespace SilKit {


//////////////////////////////////////////////////////////////////////
//Trait: associate a configuration type to the service type
//////////////////////////////////////////////////////////////////////
template <class SilKitServiceT>
struct SilKitServiceTraitConfigType
{
    // not defined by default: using type = SomeType
    using type = void;
};

template <typename ServiceT>
using SilKitServiceTraitConfigType_t = typename SilKitServiceTraitConfigType<ServiceT>::type;

#define DefineSilKitServiceTrait_ConfigType(SERVICE_NAME, CONFIG_TYPE) \
    template <> \
    struct SilKitServiceTraitConfigType<SERVICE_NAME> \
    { \
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

DefineSilKitServiceTrait_ConfigType(VSilKit::MetricsReceiver, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(VSilKit::MetricsSender, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Logging::LogMsgReceiver, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Logging::LogMsgSender, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::TimeSyncService,
                                    SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::LifecycleService,
                                    SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::SystemMonitor, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Services::Orchestration::SystemController,
                                    SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Core::Discovery::ServiceDiscovery, SilKit::Config::InternalController);
DefineSilKitServiceTrait_ConfigType(SilKit::Core::RequestReply::RequestReplyService,
                                    SilKit::Config::InternalController);


//////////////////////////////////////////////////////////////////////
//Trait: associate a ServiceType attribute with a service type
//////////////////////////////////////////////////////////////////////

template <class SilKitServiceT>
struct SilKitServiceTraitServiceType
{
    constexpr static auto GetServiceType() -> Core::ServiceType
    {
        return Core::ServiceType::Controller; // Most services are actual controllers
    }
};

#define DefineSilKitServiceTrait_ServiceType(SERVICE_NAME, SERVICE_TYPE_VALUE) \
    template <> \
    struct SilKitServiceTraitServiceType<SERVICE_NAME> \
    { \
        constexpr static auto GetServiceType() -> Core::ServiceType \
        { \
            return SERVICE_TYPE_VALUE; \
        } \
    }

DefineSilKitServiceTrait_ServiceType(SilKit::Core::Discovery::ServiceDiscovery,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::SystemController,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::SystemMonitor,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::TimeSyncService,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Orchestration::LifecycleService,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(VSilKit::MetricsReceiver, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(VSilKit::MetricsSender, SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Logging::LogMsgReceiver,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Services::Logging::LogMsgSender,
                                     SilKit::Core::ServiceType::InternalController);
DefineSilKitServiceTrait_ServiceType(SilKit::Core::RequestReply::RequestReplyService,
                                     SilKit::Core::ServiceType::InternalController);
} // namespace SilKit
