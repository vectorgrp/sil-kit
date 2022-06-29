// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFlexrayController interface
 *
 *  Used by the Participant, implemented by the FlexrayController
 */
class IIbToFlexrayController
    : public mw::IIbReceiver<FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent,
                             FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
    , public mw::IIbSender<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                           FlexrayTxBufferUpdate>
{
};

} // namespace fr
} // namespace sim
} // namespace ib
