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
