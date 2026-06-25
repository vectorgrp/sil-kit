// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "core/internal/IServiceEndpoint.hpp"
#include "wire/flexray/WireFlexrayMessages.hpp"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief IMsgForFlexrayBusSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForFlexraySimulator
    : public Core::IReceiver<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                             WireFlexrayTxBufferUpdate>
    , public Core::ISender<WireFlexrayFrameEvent, WireFlexrayFrameTransmitEvent, FlexraySymbolEvent,
                           FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
{
public:
    ~IMsgForFlexraySimulator() = default;
};

} // namespace Flexray
} // namespace Services
} // namespace SilKit
