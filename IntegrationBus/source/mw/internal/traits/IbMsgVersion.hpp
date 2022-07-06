// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <cstdint>

namespace SilKit {
namespace Core {

// the silkit messages type traits
using VersionT = uint32_t;
template <class MsgT> struct SilKitMsgTraitVersion { static constexpr VersionT Version(); };

//helper to reduce boilerplate
#define DefineSilKitMsgTrait_Version(TYPE_NAME, VERSION) \
    template<> \
    struct SilKitMsgTraitVersion<TYPE_NAME> {\
        static constexpr VersionT Version() { return VERSION; }\
    }

DefineSilKitMsgTrait_Version(SilKit::Core::Logging::LogMsg, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Orchestration::ParticipantCommand, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Orchestration::SystemCommand, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Orchestration::ParticipantStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Orchestration::WorkflowConfiguration, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Orchestration::NextSimTask, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::PubSub::DataMessageEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Rpc::FunctionCall, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Rpc::FunctionCallResponse, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanControllerStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanConfigureBaudrate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanSetControllerMode, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetSetMode, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinSendFrameRequest, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinSendFrameHeaderRequest, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinTransmission, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinWakeupPulse, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinControllerConfig, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinControllerStatusUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinFrameResponseUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexraySymbolEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexraySymbolTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayCycleStartEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayHostCommand, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayControllerConfig, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayTxBufferUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayPocStatusEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Discovery::ParticipantDiscoveryEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Discovery::ServiceDiscoveryEvent, 1);

} // namespace Core
} // namespace SilKit
