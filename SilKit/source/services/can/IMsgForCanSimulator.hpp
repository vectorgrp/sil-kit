// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireCanMessages.hpp"

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

/*! \brief IMsgForCanSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForCanSimulator
    : public Core::IReceiver<WireCanFrameEvent, CanConfigureBaudrate, CanSetControllerMode>
    , public Core::ISender<WireCanFrameEvent, CanFrameTransmitEvent, CanControllerStatus>
{
public:
    ~IMsgForCanSimulator() = default;

    /* NB: there is no setter or getter for an EndpointAddress of the bus
     * simulator, since a network simulator manages multiple controllers with
     * different endpoints. I.e., a network simulator is aware of the endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this CAN network simulator
    virtual void SetParticipantId(SilKit::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> SilKit::ParticipantId = 0;

};

} // namespace Can
} // namespace Services
} // namespace SilKit
