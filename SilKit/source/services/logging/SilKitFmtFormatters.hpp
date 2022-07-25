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

#include <fmt/format.h>
#include <type_traits>
#include <tuple>

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
#include "ServiceDatatypes.hpp"

#include "WireCanMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"
#include "WireRpcMessages.hpp"

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
        SilKit::Services::Orchestration::NextSimTask,
        SilKit::Services::Orchestration::SystemCommand,
        SilKit::Services::Orchestration::ParticipantCommand,
        SilKit::Services::Orchestration::ParticipantStatus,
        SilKit::Services::Orchestration::WorkflowConfiguration,
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

