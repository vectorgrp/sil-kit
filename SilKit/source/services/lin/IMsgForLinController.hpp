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

#include "silkit/services/lin/LinDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief IMsgForLinController interface
 *
 *  Used by the Participant, implemented by the LinControllerProxy
 */
class IMsgForLinController
    : public Core::IReceiver<LinTransmission, LinWakeupPulse, WireLinControllerConfig, LinControllerStatusUpdate,
                             LinSendFrameHeaderRequest, LinFrameResponseUpdate>
    , public Core::ISender<LinTransmission, LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse,
                           WireLinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
