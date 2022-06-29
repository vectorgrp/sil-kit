// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <cstdint>

namespace ib {
namespace mw {

// The SerdesName must not change, even when the C++ struct's name changes.
// This ensures the stability of the wire format of the vasio ser/des.

template <class MsgT> struct IbMsgTraitSerdesName { static constexpr const char* SerdesName(); };

//helper to reduce boilerplate
#define DefineIbMsgTrait_SerdesName(TYPE_NAME, SERDES_NAME) \
    template<> \
    struct IbMsgTraitSerdesName<TYPE_NAME> {\
        static constexpr const char* SerdesName() { return SERDES_NAME; }\
    }

DefineIbMsgTrait_SerdesName(ib::mw::logging::LogMsg, "LOGMSG" );
DefineIbMsgTrait_SerdesName(ib::mw::sync::ParticipantCommand, "PARTICIPANTCOMMAND" );
DefineIbMsgTrait_SerdesName(ib::mw::sync::SystemCommand, "SYSTEMCOMMAND" );
DefineIbMsgTrait_SerdesName(ib::mw::sync::ParticipantStatus, "PARTICIPANTSTATUS" );
DefineIbMsgTrait_SerdesName(ib::mw::sync::WorkflowConfiguration, "WORKFLOWCONFIGURATION" );
DefineIbMsgTrait_SerdesName(ib::mw::sync::NextSimTask, "NEXTSIMTASK" );
DefineIbMsgTrait_SerdesName(ib::sim::data::DataMessageEvent, "DATAMESSAGEEVENT" );
DefineIbMsgTrait_SerdesName(ib::sim::rpc::FunctionCall, "FUNCTIONCALL" );
DefineIbMsgTrait_SerdesName(ib::sim::rpc::FunctionCallResponse, "FUNCTIONCALLRESPONSE" );
DefineIbMsgTrait_SerdesName(ib::sim::can::CanFrameEvent, "CANFRAMEEVENT" );
DefineIbMsgTrait_SerdesName(ib::sim::can::CanFrameTransmitEvent, "CANFRAMETRANSMITEVENT" );
DefineIbMsgTrait_SerdesName(ib::sim::can::CanControllerStatus, "CANCONTROLLERSTATUS" );
DefineIbMsgTrait_SerdesName(ib::sim::can::CanConfigureBaudrate, "CANCONFIGUREBAUDRATE" );
DefineIbMsgTrait_SerdesName(ib::sim::can::CanSetControllerMode, "CANSETCONTROLLERMODE" );
DefineIbMsgTrait_SerdesName(ib::sim::eth::EthernetFrameEvent, "ETHERNETFRAMEEVENT" );
DefineIbMsgTrait_SerdesName(ib::sim::eth::EthernetFrameTransmitEvent, "ETHERNETFRAMETRANSMITEVENT" );
DefineIbMsgTrait_SerdesName(ib::sim::eth::EthernetStatus, "ETHERNETSTATUS" );
DefineIbMsgTrait_SerdesName(ib::sim::eth::EthernetSetMode, "ETHERNETSETMODE" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinSendFrameRequest, "SENDFRAMEREQUEST" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinSendFrameHeaderRequest, "SENDFRAMEHEADERREQUEST" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinTransmission, "TRANSMISSION" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinWakeupPulse, "WAKEUPPULSE" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinControllerConfig, "CONTROLLERCONFIG" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinControllerStatusUpdate, "CONTROLLERSTATUSUPDATE" );
DefineIbMsgTrait_SerdesName(ib::sim::lin::LinFrameResponseUpdate, "FRAMERESPONSEUPDATE" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayFrameEvent, "FRMESSAGE" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayFrameTransmitEvent, "FRMESSAGEACK" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexraySymbolEvent, "FRSYMBOL" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexraySymbolTransmitEvent, "FRSYMBOLACK" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayCycleStartEvent, "CYCLESTART" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayHostCommand, "HOSTCOMMAND" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayControllerConfig, "CONTROLLERCONFIG" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayTxBufferConfigUpdate, "TXBUFFERCONFIGUPDATE" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayTxBufferUpdate, "TXBUFFERUPDATE" );
DefineIbMsgTrait_SerdesName(ib::sim::fr::FlexrayPocStatusEvent, "POCSTATUS" );
DefineIbMsgTrait_SerdesName(ib::mw::service::ParticipantDiscoveryEvent, "SERVICEANNOUNCEMENT" );
DefineIbMsgTrait_SerdesName(ib::mw::service::ServiceDiscoveryEvent, "SERVICEDISCOVERYEVENT" );

} // mw
} // namespace ib
