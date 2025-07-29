// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
