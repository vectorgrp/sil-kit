// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantExtensionsImpl.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Experimental {
namespace Participant {

auto CreateSystemControllerImpl(IParticipant* participant)
    -> SilKit::Experimental::Services::Orchestration::ISystemController*
{
    auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(participant);
    if (participantInternal == nullptr)
    {
        throw SilKitError("participant is not a valid SilKit::IParticipant*");
    }
    if (participantInternal->GetIsSystemControllerCreated())
    {
        throw SilKitError("You may not create the system controller more than once.");
    }
    participantInternal->SetIsSystemControllerCreated(true);
    return participantInternal->GetSystemController();
}

auto CreateNetworkSimulatorImpl(IParticipant* participant)
    -> SilKit::Experimental::NetworkSimulation::INetworkSimulator*
{
    auto participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(participant);
    if (participantInternal == nullptr)
    {
        throw SilKitError("participant is not a valid SilKit::IParticipant*");
    }
    if (participantInternal->GetIsNetworkSimulatorCreated())
    {
        throw SilKitError("You may not create the network simulator more than once.");
    }
    participantInternal->SetIsNetworkSimulatorCreated(true);
    return participantInternal->CreateNetworkSimulator();
}

} // namespace Participant
} // namespace Experimental
} // namespace SilKit
