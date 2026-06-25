// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

#include "core/internal/string_utils_sync.hpp"
#include "core/internal/string_utils_internal.hpp"
#include "core/service/ServiceDatatypes.hpp"
#include "core/internal/LoggingDatatypesInternal.hpp"
#include "services/metrics/MetricsDatatypes.hpp"

#include "core/requests/RequestReplyDatatypes.hpp"

#include "wire/can/WireCanMessages.hpp"
#include "wire/ethernet/WireEthernetMessages.hpp"
#include "wire/flexray/WireFlexrayMessages.hpp"
#include "wire/lin/WireLinMessages.hpp"
#include "wire/rpc/WireRpcMessages.hpp"
#include "wire/pubsub/WireDataMessages.hpp"

#include "config/ParticipantConfiguration.hpp"

#include "core/internal/TestDataTypes.hpp"    // for operator<<
#include "core/internal/IServiceEndpoint.hpp" // for operator<<(... ServiceDescriptor)

#define MAKE_FORMATTER(TYPE) \
    template <> \
    struct fmt::formatter<TYPE> : ostream_formatter \
    { \
    }


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

MAKE_FORMATTER(VSilKit::MetricKind);
MAKE_FORMATTER(VSilKit::MetricsUpdate);

MAKE_FORMATTER(SilKit::Services::Orchestration::NextSimTask);
MAKE_FORMATTER(SilKit::Services::Orchestration::ParticipantState);
MAKE_FORMATTER(SilKit::Services::Orchestration::ParticipantStatus);
MAKE_FORMATTER(SilKit::Services::Orchestration::SystemState);
MAKE_FORMATTER(SilKit::Services::Orchestration::SystemCommand);
MAKE_FORMATTER(SilKit::Services::Orchestration::WorkflowConfiguration);

MAKE_FORMATTER(SilKit::Services::PubSub::WireDataMessageEvent);

MAKE_FORMATTER(SilKit::Services::Rpc::FunctionCall);
MAKE_FORMATTER(SilKit::Services::Rpc::FunctionCallResponse);

MAKE_FORMATTER(SilKit::Services::MatchingLabel::Kind);
MAKE_FORMATTER(SilKit::Services::MatchingLabel);

MAKE_FORMATTER(SilKit::Config::V1::Label::Kind);
MAKE_FORMATTER(SilKit::Config::V1::Label);

MAKE_FORMATTER(SilKit::Core::ServiceDescriptor);
MAKE_FORMATTER(SilKit::Core::ProtocolVersion);
MAKE_FORMATTER(SilKit::Core::Discovery::ParticipantDiscoveryEvent);
MAKE_FORMATTER(SilKit::Core::Discovery::ServiceDiscoveryEvent);
MAKE_FORMATTER(SilKit::Core::Tests::TestMessage);
MAKE_FORMATTER(SilKit::Core::RequestReply::RequestReplyCall);
MAKE_FORMATTER(SilKit::Core::RequestReply::RequestReplyCallReturn);
