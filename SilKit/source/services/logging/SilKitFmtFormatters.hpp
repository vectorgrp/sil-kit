// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>
#include <type_traits>
#include <tuple>

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/logging/string_utils.hpp"
#include "silkit/core/sync/string_utils.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/eth/string_utils.hpp"
#include "silkit/services/fr/string_utils.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/pubsub/string_utils.hpp"
#include "silkit/services/rpc/string_utils.hpp"

#include "string_utils_sync.hpp"
#include "ServiceDatatypes.hpp"

// Template helpers
template<class Query, class...List>
struct is_one_of : std::false_type 
{
};

template<class Query, class...List>
static constexpr bool is_one_of_v = is_one_of<Query, List...>::value;

template<class Query, class E0, class...List>
struct is_one_of<Query, E0, List...> : std::integral_constant<bool, std::is_same<Query, E0>::value || is_one_of_v<Query, List...>>
{
};
template<class T, class = void>
struct is_printable_silkit_type : std::false_type 
{
};
template<class T>
static constexpr bool is_printable_silkit_type_v = is_printable_silkit_type<T>::value;

template<class T>
struct is_printable_silkit_type<T, std::enable_if_t<is_one_of_v<T, 
        SilKit::Services::Logging::LogMsg,
        SilKit::Core::Orchestration::NextSimTask,
        SilKit::Core::Orchestration::SystemCommand,
        SilKit::Core::Orchestration::ParticipantCommand,
        SilKit::Core::Orchestration::ParticipantStatus,
        SilKit::Core::Orchestration::WorkflowConfiguration,
        SilKit::Services::PubSub::DataMessageEvent,
        SilKit::Services::Rpc::FunctionCall,
        SilKit::Services::Rpc::FunctionCallResponse,
        SilKit::Services::Can::CanFrame,
        SilKit::Services::Can::CanFrameEvent,
        SilKit::Services::Can::CanFrameTransmitEvent,
        SilKit::Services::Can::CanControllerStatus,
        SilKit::Services::Can::CanConfigureBaudrate,
        SilKit::Services::Can::CanSetControllerMode,
        SilKit::Services::Ethernet::EthernetFrameEvent,
        SilKit::Services::Ethernet::EthernetFrameTransmitEvent,
        SilKit::Services::Ethernet::EthernetStatus,
        SilKit::Services::Ethernet::EthernetSetMode,
        SilKit::Services::Lin::LinSendFrameRequest,
        SilKit::Services::Lin::LinSendFrameHeaderRequest,
        SilKit::Services::Lin::LinTransmission,
        SilKit::Services::Lin::LinWakeupPulse,
        SilKit::Services::Lin::LinControllerConfig,
        SilKit::Services::Lin::LinControllerStatusUpdate,
        SilKit::Services::Lin::LinFrameResponseUpdate,
        SilKit::Services::Flexray::FlexrayFrameEvent,
        SilKit::Services::Flexray::FlexrayFrameTransmitEvent,
        SilKit::Services::Flexray::FlexraySymbolEvent,
        SilKit::Services::Flexray::FlexraySymbolTransmitEvent,
        SilKit::Services::Flexray::FlexrayCycleStartEvent,
        SilKit::Services::Flexray::FlexrayHostCommand,
        SilKit::Services::Flexray::FlexrayControllerConfig,
        SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate,
        SilKit::Services::Flexray::FlexrayTxBufferUpdate,
        SilKit::Services::Flexray::FlexrayPocStatusEvent,
        SilKit::Core::Discovery::ParticipantDiscoveryEvent
        >
    >
>
    : std::true_type 
{
};

template<typename T>
struct fmt::formatter<T, typename is_printable_silkit_type<T>::type> : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const T& silkitValue, FormatContext& ctx)
    {
        std::ostringstream oss;
        oss << silkitValue;
        return formatter<string_view>::format(oss.str(), ctx);
    }
};

