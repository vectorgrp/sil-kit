// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"

#include "silkit/services/fr/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief IMsgForFlexrayBusSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForFlexrayBusSimulator
    : public Core::IReceiver<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                             FlexrayTxBufferUpdate>
    , public Core::ISender<FlexrayFrameEvent, FlexrayFrameTransmitEvent, FlexraySymbolEvent, FlexraySymbolTransmitEvent,
                           FlexrayCycleStartEvent, FlexrayPocStatusEvent>
{
public:
    ~IMsgForFlexrayBusSimulator() = default;

    /* NB: There is no setter or getter for an EndpointAddress of the bus
     * simulator, since the network simulator manages multiple controllers with
     * different endpoints. I.e., the network simulator is aware of the endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this FlexRay network simulator
    virtual void SetParticipantId(SilKit::Core::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> SilKit::Core::ParticipantId = 0;
};

} // namespace Flexray
} // namespace Services
} // namespace SilKit
