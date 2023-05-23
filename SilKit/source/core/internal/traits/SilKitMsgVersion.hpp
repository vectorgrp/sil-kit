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

DefineSilKitMsgTrait_Version(SilKit::Services::Logging::LogMsg, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Orchestration::SystemCommand, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Orchestration::ParticipantStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Orchestration::WorkflowConfiguration, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Orchestration::NextSimTask, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::PubSub::WireDataMessageEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Rpc::FunctionCall, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Rpc::FunctionCallResponse, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::WireCanFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanControllerStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanConfigureBaudrate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Can::CanSetControllerMode, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::WireEthernetFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetStatus, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Ethernet::EthernetSetMode, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinSendFrameRequest, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinSendFrameHeaderRequest, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinTransmission, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinWakeupPulse, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::WireLinControllerConfig, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinControllerStatusUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Lin::LinFrameResponseUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::WireFlexrayFrameEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::WireFlexrayFrameTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexraySymbolEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexraySymbolTransmitEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayCycleStartEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayHostCommand, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayControllerConfig, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::WireFlexrayTxBufferUpdate, 1);
DefineSilKitMsgTrait_Version(SilKit::Services::Flexray::FlexrayPocStatusEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Discovery::ParticipantDiscoveryEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::Discovery::ServiceDiscoveryEvent, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::RequestReply::RequestReplyCall, 1);
DefineSilKitMsgTrait_Version(SilKit::Core::RequestReply::RequestReplyCallReturn, 1);

} // namespace Core
} // namespace SilKit
