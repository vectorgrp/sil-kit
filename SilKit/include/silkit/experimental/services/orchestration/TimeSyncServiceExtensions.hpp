// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/services/orchestration/TimeSyncDatatypesExtensions.hpp"
#include "silkit/services/orchestration/ITimeSyncService.hpp"

#include "silkit/detail/macros.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Services {
namespace Orchestration {

/*! \brief Add a \ref OtherSimulationStepsCompletedHandler on a given
 *         time sync. service for external simulation step coupling.
 *
 * \param timeSyncService The time sync. service.
 * \param handler The handler to be called when completion of the current sim. step will immediately progress sim. time.
 *
 * \return The handler identifier that can be used to remove the callback.
 */
DETAIL_SILKIT_CPP_API auto AddOtherSimulationStepsCompletedHandler(
    SilKit::Services::Orchestration::ITimeSyncService* timeSyncService,
    SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler handler)
    -> SilKit::Util::HandlerId;

/*! \brief Remove a \ref SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler by
 *         HandlerId on a given controller.
 *
 * \param timeSyncService The time sync. service.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 */
DETAIL_SILKIT_CPP_API void RemoveOtherSimulationStepsCompletedHandler(
    SilKit::Services::Orchestration::ITimeSyncService* timeSyncService, SilKit::Util::HandlerId handlerId);

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


//! \cond DOCUMENT_HEADER_ONLY_DETAILS
#include "silkit/detail/impl/experimental/services/orchestration/TimeSyncServiceExtensions.ipp"
//! \endcond
