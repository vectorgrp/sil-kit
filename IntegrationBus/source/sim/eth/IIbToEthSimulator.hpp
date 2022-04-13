// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "IIbServiceEndpoint.hpp"
#include "ib/sim/eth/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace eth {

/*! \brief IIbToEthSimulator interface
*
*  Used by the Participant, implemented by the EthSimulator
*/
class IIbToEthSimulator
    : public mw::IIbReceiver<EthMessage, EthSetMode>
    , public mw::IIbSender<EthMessage, EthTransmitAcknowledge, EthStatus>
{
public:
    virtual ~IIbToEthSimulator() = default;
    
    /* NB: there is no setter or getter for an EndpointAddress of the
     * simulator, since the simulator manages multiple controllers
     * with different endpoints. I.e., the simulator is aware of all
     * the individual endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this ethernetsimulator
    virtual void SetParticipantId(ib::mw::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> ib::mw::ParticipantId = 0;
};

} // namespace eth
} // namespace sim
} // namespace ib
