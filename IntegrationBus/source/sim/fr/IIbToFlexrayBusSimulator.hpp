// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "IIbServiceEndpoint.hpp"

#include "ib/sim/fr/FlexrayDatatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief IIbToFlexrayBusSimulator interface
 *
 *  Used by the Participant
 */
class IIbToFlexrayBusSimulator
    : public mw::IIbReceiver<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                             FlexrayTxBufferUpdate>
    , public mw::IIbSender<FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent, FlexraySymbolTransmitEvent,
                           FlexrayCycleStartEvent, FlexrayPocStatusEvent>
{
public:
    ~IIbToFlexrayBusSimulator() = default;

    /* NB: There is no setter or getter for an EndpointAddress of the bus
     * simulator, since the Network Simulator manages multiple controllers with
     * different endpoints. I.e., the Network Simulator is aware of the endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this CAN Network Simulator
    virtual void SetParticipantId(ib::mw::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> ib::mw::ParticipantId = 0;
};

} // namespace fr
} // namespace sim
} // namespace ib
