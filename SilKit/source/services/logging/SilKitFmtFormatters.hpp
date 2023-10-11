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

#include <fmt/ostream.h>

// Include all operator<<(std::ostream&...) for libfmt here
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/logging/string_utils.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/pubsub/string_utils.hpp"
#include "silkit/services/rpc/string_utils.hpp"

#include "string_utils_sync.hpp"
#include "string_utils_internal.hpp"
#include "ServiceDatatypes.hpp"
#include "LoggingDatatypesInternal.hpp"

#include "RequestReplyDatatypes.hpp"

#include "WireCanMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"
#include "WireRpcMessages.hpp"
#include "WireDataMessages.hpp"

#include "TestDataTypes.hpp" // for operator<<
#include "IServiceEndpoint.hpp" // for operator<<(... ServiceDescriptor)


#define MAKE_FORMATTER(TYPE) template<> struct fmt::formatter<TYPE> : ostream_formatter {}


MAKE_FORMATTER(SilKit::Services::Can::CanConfigureBaudrate);
MAKE_FORMATTER(SilKit::Services::Can::CanControllerStatus);
MAKE_FORMATTER(SilKit::Services::Can::CanFrameTransmitEvent);
MAKE_FORMATTER(SilKit::Services::Can::CanSetControllerMode);
MAKE_FORMATTER(SilKit::Services::Can::WireCanFrameEvent);

MAKE_FORMATTER(SilKit::Services::Ethernet::EthernetFrameTransmitEvent);
MAKE_FORMATTER(SilKit::Services::Ethernet::EthernetSetMode);
MAKE_FORMATTER(SilKit::Services::Ethernet::EthernetStatus);
MAKE_FORMATTER(SilKit::Services::Ethernet::WireEthernetFrameEvent);

MAKE_FORMATTER(SilKit::Services::Flexray::FlexrayControllerConfig);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexrayCycleStartEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexrayHostCommand);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexrayPocStatusEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexraySymbolEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexraySymbolTransmitEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate);
MAKE_FORMATTER(SilKit::Services::Flexray::WireFlexrayFrameEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::WireFlexrayFrameTransmitEvent);
MAKE_FORMATTER(SilKit::Services::Flexray::WireFlexrayTxBufferUpdate);

MAKE_FORMATTER(SilKit::Services::Lin::LinChecksumModel);
MAKE_FORMATTER(SilKit::Services::Lin::LinControllerConfig);
MAKE_FORMATTER(SilKit::Services::Lin::LinControllerMode);
MAKE_FORMATTER(SilKit::Services::Lin::LinControllerStatus);
MAKE_FORMATTER(SilKit::Services::Lin::LinControllerStatusUpdate);
MAKE_FORMATTER(SilKit::Services::Lin::LinFrameResponseMode);
MAKE_FORMATTER(SilKit::Services::Lin::LinFrameResponseUpdate);
MAKE_FORMATTER(SilKit::Services::Lin::LinSendFrameHeaderRequest);
MAKE_FORMATTER(SilKit::Services::Lin::LinSendFrameRequest);
MAKE_FORMATTER(SilKit::Services::Lin::LinTransmission);
MAKE_FORMATTER(SilKit::Services::Lin::LinWakeupPulse);
MAKE_FORMATTER(SilKit::Services::Lin::WireLinControllerConfig);

MAKE_FORMATTER(SilKit::Services::Logging::LogMsg);

MAKE_FORMATTER(SilKit::Services::Orchestration::NextSimTask);
MAKE_FORMATTER(SilKit::Services::Orchestration::ParticipantState);
MAKE_FORMATTER(SilKit::Services::Orchestration::ParticipantStatus);
MAKE_FORMATTER(SilKit::Services::Orchestration::SystemState);
MAKE_FORMATTER(SilKit::Services::Orchestration::SystemCommand);
MAKE_FORMATTER(SilKit::Services::Orchestration::WorkflowConfiguration);

MAKE_FORMATTER(SilKit::Services::PubSub::WireDataMessageEvent);

MAKE_FORMATTER(SilKit::Services::Rpc::FunctionCall);
MAKE_FORMATTER(SilKit::Services::Rpc::FunctionCallResponse);


MAKE_FORMATTER(SilKit::Core::ServiceDescriptor);
MAKE_FORMATTER(SilKit::Core::ProtocolVersion);
MAKE_FORMATTER(SilKit::Core::Discovery::ParticipantDiscoveryEvent);
MAKE_FORMATTER(SilKit::Core::Discovery::ServiceDiscoveryEvent);
MAKE_FORMATTER(SilKit::Core::Tests::TestMessage);
MAKE_FORMATTER(SilKit::Core::RequestReply::RequestReplyCall);
MAKE_FORMATTER(SilKit::Core::RequestReply::RequestReplyCallReturn);
