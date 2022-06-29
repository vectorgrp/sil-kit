// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <cstdint>

namespace ib {
namespace mw {

// the ib messages type traits
using VersionT = uint32_t;
template <class MsgT> struct IbMsgTraitVersion { static constexpr VersionT Version(); };

//helper to reduce boilerplate
#define DefineIbMsgTrait_Version(TYPE_NAME, VERSION) \
    template<> \
    struct IbMsgTraitVersion<TYPE_NAME> {\
        static constexpr VersionT Version() { return VERSION; }\
    }

DefineIbMsgTrait_Version(ib::mw::logging::LogMsg, 1);
DefineIbMsgTrait_Version(ib::mw::sync::ParticipantCommand, 1);
DefineIbMsgTrait_Version(ib::mw::sync::SystemCommand, 1);
DefineIbMsgTrait_Version(ib::mw::sync::ParticipantStatus, 1);
DefineIbMsgTrait_Version(ib::mw::sync::WorkflowConfiguration, 1);
DefineIbMsgTrait_Version(ib::mw::sync::NextSimTask, 1);
DefineIbMsgTrait_Version(ib::sim::data::DataMessageEvent, 1);
DefineIbMsgTrait_Version(ib::sim::rpc::FunctionCall, 1);
DefineIbMsgTrait_Version(ib::sim::rpc::FunctionCallResponse, 1);
DefineIbMsgTrait_Version(ib::sim::can::CanFrameEvent, 1);
DefineIbMsgTrait_Version(ib::sim::can::CanFrameTransmitEvent, 1);
DefineIbMsgTrait_Version(ib::sim::can::CanControllerStatus, 1);
DefineIbMsgTrait_Version(ib::sim::can::CanConfigureBaudrate, 1);
DefineIbMsgTrait_Version(ib::sim::can::CanSetControllerMode, 1);
DefineIbMsgTrait_Version(ib::sim::eth::EthernetFrameEvent, 1);
DefineIbMsgTrait_Version(ib::sim::eth::EthernetFrameTransmitEvent, 1);
DefineIbMsgTrait_Version(ib::sim::eth::EthernetStatus, 1);
DefineIbMsgTrait_Version(ib::sim::eth::EthernetSetMode, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinSendFrameRequest, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinSendFrameHeaderRequest, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinTransmission, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinWakeupPulse, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinControllerConfig, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinControllerStatusUpdate, 1);
DefineIbMsgTrait_Version(ib::sim::lin::LinFrameResponseUpdate, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayFrameEvent, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayFrameTransmitEvent, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexraySymbolEvent, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexraySymbolTransmitEvent, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayCycleStartEvent, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayHostCommand, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayControllerConfig, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayTxBufferConfigUpdate, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayTxBufferUpdate, 1);
DefineIbMsgTrait_Version(ib::sim::fr::FlexrayPocStatusEvent, 1);
DefineIbMsgTrait_Version(ib::mw::service::ParticipantDiscoveryEvent, 1);
DefineIbMsgTrait_Version(ib::mw::service::ServiceDiscoveryEvent, 1);

} // mw
} // namespace ib
