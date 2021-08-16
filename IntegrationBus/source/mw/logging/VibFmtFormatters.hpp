// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/logging/ILogger.hpp"

#include "ib/mw/string_utils.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/string_utils.hpp"
#include "ib/sim/eth/string_utils.hpp"
#include "ib/sim/fr/string_utils.hpp"
#include "ib/sim/lin/string_utils.hpp"
#include "ib/sim/io/string_utils.hpp"
#include "ib/sim/generic/string_utils.hpp"

#include <fmt/format.h>

#include <type_traits>
#include <tuple>


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
        ib::mw::sync::Tick,
        ib::mw::sync::TickDone,
        ib::mw::sync::QuantumRequest,
        ib::mw::sync::QuantumGrant,
        ib::mw::sync::SystemCommand,
        ib::mw::sync::ParticipantCommand,
        ib::mw::sync::ParticipantStatus,
        ib::sim::generic::GenericMessage,
        ib::sim::can::CanMessage,
        ib::sim::can::CanTransmitAcknowledge,
        ib::sim::can::CanControllerStatus,
        ib::sim::can::CanConfigureBaudrate,
        ib::sim::can::CanSetControllerMode,
        ib::sim::eth::EthMessage,
        ib::sim::eth::EthTransmitAcknowledge,
        ib::sim::eth::EthStatus,
        ib::sim::eth::EthSetMode,
        ib::sim::io::AnalogIoMessage,
        ib::sim::io::DigitalIoMessage,
        ib::sim::io::PatternIoMessage,
        ib::sim::io::PwmIoMessage,
        ib::sim::lin::SendFrameRequest,
        ib::sim::lin::SendFrameHeaderRequest,
        ib::sim::lin::Transmission,
        ib::sim::lin::WakeupPulse,
        ib::sim::lin::ControllerConfig,
        ib::sim::lin::ControllerStatusUpdate,
        ib::sim::lin::FrameResponseUpdate,
        ib::sim::fr::FrMessage,
        ib::sim::fr::FrMessageAck,
        ib::sim::fr::FrSymbol,
        ib::sim::fr::FrSymbolAck,
        ib::sim::fr::CycleStart,
        ib::sim::fr::HostCommand,
        ib::sim::fr::ControllerConfig,
        ib::sim::fr::TxBufferConfigUpdate,
        ib::sim::fr::TxBufferUpdate,
        ib::sim::fr::ControllerStatus,
        ib::sim::fr::PocStatus
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

