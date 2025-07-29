// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/detail/impl/services/orchestration/TimeSyncService.hpp"
#include "silkit/experimental/services/orchestration/TimeSyncDatatypesExtensions.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Services {
namespace Orchestration {

auto AddOtherSimulationStepsCompletedHandler(
    SilKit::Services::Orchestration::ITimeSyncService* cppITimeSyncService,
    SilKit::Experimental::Services::Orchestration::OtherSimulationStepsCompletedHandler handler)
    -> SilKit::Util::HandlerId
{
    auto& cppTimeSyncService = dynamic_cast<Impl::Services::Orchestration::TimeSyncService&>(*cppITimeSyncService);

    return cppTimeSyncService.ExperimentalAddOtherSimulationStepsCompletedHandler(handler);
}

void RemoveOtherSimulationStepsCompletedHandler(SilKit::Services::Orchestration::ITimeSyncService* cppITimeSyncService,
                                                SilKit::Util::HandlerId handlerId)
{
    auto& cppTimeSyncService = dynamic_cast<Impl::Services::Orchestration::TimeSyncService&>(*cppITimeSyncService);

    return cppTimeSyncService.ExperimentalRemoveOtherSimulationStepsCompletedHandler(handlerId);
}

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Orchestration::
    AddOtherSimulationStepsCompletedHandler;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Services::Orchestration::
    RemoveOtherSimulationStepsCompletedHandler;
} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit
