/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireFlexrayMessages.hpp"

#include "silkit/services/flexray/FlexrayDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief IMsgForFlexrayBusSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForFlexraySimulator
    : public Core::IReceiver<FlexrayHostCommand, FlexrayControllerConfig, FlexrayTxBufferConfigUpdate,
                             WireFlexrayTxBufferUpdate>
    , public Core::ISender<WireFlexrayFrameEvent, WireFlexrayFrameTransmitEvent, FlexraySymbolEvent,
                           FlexraySymbolTransmitEvent, FlexrayCycleStartEvent, FlexrayPocStatusEvent>
{
public:
    ~IMsgForFlexraySimulator() = default;

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
