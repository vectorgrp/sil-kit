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
#include "silkit/experimental/netsim/INetworkSimulator.hpp"

#include "silkit/detail/impl/participant/Participant.hpp"
#include "silkit/detail/impl/experimental/services/orchestration/SystemController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Experimental {
namespace Participant {

auto CreateSystemController(SilKit::IParticipant* cppIParticipant)
    -> SilKit::Experimental::Services::Orchestration::ISystemController*
{
    auto& cppParticipant = dynamic_cast<Impl::Participant&>(*cppIParticipant);

    return cppParticipant.ExperimentalCreateSystemController();
}

auto CreateNetworkSimulator(SilKit::IParticipant* cppIParticipant)
    -> SilKit::Experimental::NetworkSimulation::INetworkSimulator*
{
    auto& cppParticipant = dynamic_cast<Impl::Participant&>(*cppIParticipant);

    return cppParticipant.ExperimentalCreateNetworkSimulator();
}

} // namespace Participant
} // namespace Experimental
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Experimental {
namespace Participant {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Participant::CreateSystemController;
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Experimental::Participant::CreateNetworkSimulator;
} // namespace Participant
} // namespace Experimental
} // namespace SilKit
