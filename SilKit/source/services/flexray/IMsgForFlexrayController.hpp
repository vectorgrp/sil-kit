// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "WireFlexrayMessages.hpp"

#include "silkit/services/flexray/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief IMsgForFlexrayController interface
 *
 *  Used by the Participant, implemented by the FlexrayController
 */
class IMsgForFlexrayController
    : public Core::IReceiver<WireFlexrayFrameEvent, WireFlexrayFrameTransmitEvent, FlexraySymbolEvent,
                             FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
    , public Core::ISender<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                           WireFlexrayTxBufferUpdate>
{
};

} // namespace Flexray
} // namespace Services
} // namespace SilKit
