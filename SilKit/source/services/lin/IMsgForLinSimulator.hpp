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

#include "silkit/services/lin/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief IMsgForLinSimulator interface
*
*  Used by the Participant, implemented by the LinSimulator
*/
class IMsgForLinSimulator
    : public Core::IReceiver<LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse, WireLinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
    , public Core::ISender<LinSendFrameHeaderRequest, LinTransmission, LinWakeupPulse, WireLinControllerConfig>
{
public:
    virtual ~IMsgForLinSimulator() = default;
    
    /* NB: there is no setter or getter for an EndpointAddress of the
     * simulator, since the simulator manages multiple controllers
     * with different endpoints. I.e., the simulator is aware of all
     * the individual endpointIds.
     */
    //! \brief Setter and getter for the ParticipantID associated with this LIN simulator
    virtual void SetParticipantId(SilKit::Core::ParticipantId participantId) = 0;
    virtual auto GetParticipantId() const -> SilKit::Core::ParticipantId = 0;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
