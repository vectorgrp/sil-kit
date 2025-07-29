// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cstdint>

namespace SilKit {
namespace Core {

// The SerdesName must not change, even when the C++ struct's name changes.
// This ensures the stability of the wire format of the vasio ser/des.

template <class MsgT>
struct SilKitMsgTraitSerdesName
{
    static constexpr const char* SerdesName();
};

//helper to reduce boilerplate
#define DefineSilKitMsgTrait_SerdesName(TYPE_NAME, SERDES_NAME) \
    template <> \
    struct SilKitMsgTraitSerdesName<TYPE_NAME> \
    { \
        static constexpr const char* SerdesName() \
        { \
            return SERDES_NAME; \
        } \
    }

DefineSilKitMsgTrait_SerdesName(SilKit::Services::Logging::LogMsg, "LOGMSG");
DefineSilKitMsgTrait_SerdesName(VSilKit::MetricsUpdate, "METRICSUPDATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Orchestration::SystemCommand, "SYSTEMCOMMAND");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Orchestration::ParticipantStatus, "PARTICIPANTSTATUS");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Orchestration::WorkflowConfiguration, "WORKFLOWCONFIGURATION");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Orchestration::NextSimTask, "NEXTSIMTASK");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::PubSub::WireDataMessageEvent, "DATAMESSAGEEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Rpc::FunctionCall, "FUNCTIONCALL");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Rpc::FunctionCallResponse, "FUNCTIONCALLRESPONSE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Can::WireCanFrameEvent, "CANFRAMEEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Can::CanFrameTransmitEvent, "CANFRAMETRANSMITEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Can::CanControllerStatus, "CANCONTROLLERSTATUS");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Can::CanConfigureBaudrate, "CANCONFIGUREBAUDRATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Can::CanSetControllerMode, "CANSETCONTROLLERMODE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Ethernet::WireEthernetFrameEvent, "ETHERNETFRAMEEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Ethernet::EthernetFrameTransmitEvent, "ETHERNETFRAMETRANSMITEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Ethernet::EthernetStatus, "ETHERNETSTATUS");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Ethernet::EthernetSetMode, "ETHERNETSETMODE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinSendFrameRequest, "SENDFRAMEREQUEST");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinSendFrameHeaderRequest, "SENDFRAMEHEADERREQUEST");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinTransmission, "TRANSMISSION");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinWakeupPulse, "WAKEUPPULSE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::WireLinControllerConfig, "CONTROLLERCONFIG");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinControllerStatusUpdate, "CONTROLLERSTATUSUPDATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Lin::LinFrameResponseUpdate, "FRAMERESPONSEUPDATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::WireFlexrayFrameEvent, "FRMESSAGE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::WireFlexrayFrameTransmitEvent, "FRMESSAGEACK");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexraySymbolEvent, "FRSYMBOL");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexraySymbolTransmitEvent, "FRSYMBOLACK");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexrayCycleStartEvent, "CYCLESTART");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexrayHostCommand, "HOSTCOMMAND");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexrayControllerConfig, "CONTROLLERCONFIG");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate, "TXBUFFERCONFIGUPDATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::WireFlexrayTxBufferUpdate, "TXBUFFERUPDATE");
DefineSilKitMsgTrait_SerdesName(SilKit::Services::Flexray::FlexrayPocStatusEvent, "POCSTATUS");
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Discovery::ParticipantDiscoveryEvent, "SERVICEANNOUNCEMENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Discovery::ServiceDiscoveryEvent, "SERVICEDISCOVERYEVENT");
DefineSilKitMsgTrait_SerdesName(SilKit::Core::RequestReply::RequestReplyCall, "REQUESTREPLYCALL");
DefineSilKitMsgTrait_SerdesName(SilKit::Core::RequestReply::RequestReplyCallReturn, "REQUESTREPLYCALLRETURN");

} // namespace Core
} // namespace SilKit
