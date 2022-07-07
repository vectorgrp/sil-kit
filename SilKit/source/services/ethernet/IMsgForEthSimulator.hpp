// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "silkit/services/eth/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {

/*! \brief IMsgForEthSimulator interface
*
*  Used by the Participant, implemented by the EthSimulator
*/
class IMsgForEthSimulator
    : public Core::IReceiver<EthernetFrameEvent, EthernetSetMode>
    , public Core::ISender<EthernetFrameEvent, EthernetFrameTransmitEvent, EthernetStatus>
{
public:
    virtual ~IMsgForEthSimulator() = default;
    
    /* NB: there is no setter or getter for an EndpointAddress of the
     * simulator, since the simulator manages multiple controllers
     * with different endpoints. I.e., the simulator is aware of all
     * the individual endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this ethernetsimulator
    virtual void SetParticipantId(SilKit::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> SilKit::ParticipantId = 0;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
