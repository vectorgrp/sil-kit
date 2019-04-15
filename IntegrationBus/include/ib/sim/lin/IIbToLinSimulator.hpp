// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbReceiver.hpp"
#include "ib/mw/IIbSender.hpp"
#include "ib/sim/lin/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinSimulator interface
*
*  Used by the ComAdapter, implemented by the LinSimulator
*/
class IIbToLinSimulator
    : public mw::IIbReceiver<LinMessage, RxRequest, WakeupRequest, ControllerConfig, SlaveConfiguration, SlaveResponse>
    , public mw::IIbSender<LinMessage, TxAcknowledge, WakeupRequest>
{
public:
    virtual ~IIbToLinSimulator() = default;
    
    /* NB: there is no setter or getter for an EndpointAddress of the
     * simulator, since the simulator manages multiple controllers
     * with different endpoints. I.e., the simulator is aware of all
     * the individual endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this LIN simulator
    virtual void SetParticipantId(mw::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> mw::ParticipantId = 0;
};

} // namespace lin
} // namespace sim
} // namespace ib
