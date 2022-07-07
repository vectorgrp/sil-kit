// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"

#include "silkit/services/flexray/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief IMsgForFlexrayController interface
 *
 *  Used by the Participant, implemented by the FlexrayController
 */
class IMsgForFlexrayController
    : public Core::IReceiver<FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent,
                             FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
    , public Core::ISender<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                           FlexrayTxBufferUpdate>
{
};

} // namespace Flexray
} // namespace Services
} // namespace SilKit
