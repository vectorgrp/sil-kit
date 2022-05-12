// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/fr/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFlexrayControllerFacade interface
 *
 *  Used by the Participant, implemented by the FlexrayControllerFacade
 */
class IIbToFlexrayControllerFacade
    : public mw::IIbReceiver<FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent,
                             FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
    , public mw::IIbSender</*Fr*/ FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent,
                           FlexraySymbolTransmitEvent, /*FrProxy*/ FlexrayHostCommand, FlexrayControllerConfig,
                           FlexrayTxBufferConfigUpdate, FlexrayTxBufferUpdate>
{
public:
    virtual ~IIbToFlexrayControllerFacade() noexcept = default;
};

} // namespace fr
} // namespace sim
} // namespace ib
