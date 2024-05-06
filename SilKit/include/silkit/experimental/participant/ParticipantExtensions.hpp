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

#include "silkit/SilKitMacros.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "silkit/detail/macros.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Participant {

/*! \brief Return the experimental ISystemController at a given SIL Kit participant.
*
* \param participant The participant instance for which the system controller is created
*
* \throw SilKit::SilKitError The participant is invalid.
*/
DETAIL_SILKIT_CPP_API auto CreateSystemController(SilKit::IParticipant* participant)
    -> SilKit::Experimental::Services::Orchestration::ISystemController*;

DETAIL_SILKIT_CPP_API auto CreateNetworkSimulator(SilKit::IParticipant* participant)
    -> SilKit::Experimental::NetworkSimulation::INetworkSimulator*;


} // namespace Participant
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/experimental/participant/ParticipantExtensions.ipp"
//! \endcond
