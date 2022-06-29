// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <fmt/format.h>
#include <type_traits>
#include <tuple>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/logging/string_utils.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/eth/string_utils.hpp"
#include "ib/sim/fr/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/sim/data/string_utils.hpp"
#include "ib/sim/rpc/string_utils.hpp"

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
struct is_printable_vib_type : std::false_type 
{
};
template<class T>
static constexpr bool is_printable_vib_type_v = is_printable_vib_type<T>::value;

template<class T>
struct is_printable_vib_type<T, std::enable_if_t<is_one_of_v<T, 
        ib::mw::logging::LogMsg,
        ib::mw::sync::NextSimTask,
        ib::mw::sync::SystemCommand,
        ib::mw::sync::ParticipantCommand,
        ib::mw::sync::ParticipantStatus,
        ib::mw::sync::WorkflowConfiguration,
        ib::sim::data::DataMessageEvent,
        ib::sim::rpc::FunctionCall,
        ib::sim::rpc::FunctionCallResponse,
        ib::sim::can::CanFrame,
        ib::sim::can::CanFrameEvent,
        ib::sim::can::CanFrameTransmitEvent,
        ib::sim::can::CanControllerStatus,
        ib::sim::can::CanConfigureBaudrate,
        ib::sim::can::CanSetControllerMode,
        ib::sim::eth::EthernetFrameEvent,
        ib::sim::eth::EthernetFrameTransmitEvent,
        ib::sim::eth::EthernetStatus,
        ib::sim::eth::EthernetSetMode,
        ib::sim::lin::LinSendFrameRequest,
        ib::sim::lin::LinSendFrameHeaderRequest,
        ib::sim::lin::LinTransmission,
        ib::sim::lin::LinWakeupPulse,
        ib::sim::lin::LinControllerConfig,
        ib::sim::lin::LinControllerStatusUpdate,
        ib::sim::lin::LinFrameResponseUpdate,
        ib::sim::fr::FlexrayFrameEvent,
        ib::sim::fr::FlexrayFrameTransmitEvent,
        ib::sim::fr::FlexraySymbolEvent,
        ib::sim::fr::FlexraySymbolTransmitEvent,
        ib::sim::fr::FlexrayCycleStartEvent,
        ib::sim::fr::FlexrayHostCommand,
        ib::sim::fr::FlexrayControllerConfig,
        ib::sim::fr::FlexrayTxBufferConfigUpdate,
        ib::sim::fr::FlexrayTxBufferUpdate,
        ib::sim::fr::FlexrayPocStatusEvent,
        ib::mw::service::ParticipantDiscoveryEvent
        >
    >
>
    : std::true_type 
{
};

template<typename T>
struct fmt::formatter<T, typename is_printable_vib_type<T>::type> : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const T& vibValue, FormatContext& ctx)
    {
        std::ostringstream oss;
        oss << vibValue;
        return formatter<string_view>::format(oss.str(), ctx);
    }
};

