// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "IIbServiceEndpoint.hpp"

#include "ib/sim/can/CanDatatypes.hpp"

namespace ib {
namespace sim {
namespace can {

/*! \brief IIbToCanSimulator interface
 *
 *  Used by the Participant
 */
class IIbToCanSimulator
    : public mw::IIbReceiver<CanFrameEvent, CanConfigureBaudrate, CanSetControllerMode>
    , public mw::IIbSender<CanFrameEvent, CanFrameTransmitEvent, CanControllerStatus>
{
public:
    ~IIbToCanSimulator() = default;

    /* NB: there is no setter or getter for an EndpointAddress of the bus
     * simulator, since a network simulator manages multiple controllers with
     * different endpoints. I.e., a network simulator is aware of the endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this CAN network simulator
    virtual void SetParticipantId(ib::mw::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> ib::mw::ParticipantId = 0;

};

} // namespace can
} // namespace sim
} // namespace ib
