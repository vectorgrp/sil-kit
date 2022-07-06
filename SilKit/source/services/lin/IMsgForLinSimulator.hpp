// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"

#include "silkit/services/lin/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief IMsgForLinSimulator interface
*
*  Used by the Participant, implemented by the LinSimulator
*/
class IMsgForLinSimulator
    : public Core::IReceiver<LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
    , public Core::ISender<LinTransmission, LinWakeupPulse, LinControllerConfig, LinFrameResponseUpdate>
{
public:
    virtual ~IMsgForLinSimulator() = default;
    
    /* NB: there is no setter or getter for an EndpointAddress of the
     * simulator, since the simulator manages multiple controllers
     * with different endpoints. I.e., the simulator is aware of all
     * the individual endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this LIN simulator
    virtual void SetParticipantId(ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> ParticipantId = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
