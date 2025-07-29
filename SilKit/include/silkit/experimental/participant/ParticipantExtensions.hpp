// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
