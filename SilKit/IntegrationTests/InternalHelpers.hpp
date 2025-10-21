// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IParticipantInternal.hpp"


namespace SilKit {
namespace Tests {

inline auto ToParticipantInternal(SilKit::IParticipant& participant) -> SilKit::Core::IParticipantInternal&
{
    // Try casting directly into IParticipantInternal, will result in nullptr for hourglass-participants.
    auto* participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal*>(&participant);
    if (participantInternal != nullptr)
    {
        return *participantInternal;
    }

    using HourglassParticipant = SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Participant;

    // Try casting into the concrete hourglass participant, then get the C-API pointer (which is the internal
    // IParticipant pointer), then try casting to IParticipantInternal.
    auto* hourglassParticipant = dynamic_cast<HourglassParticipant*>(&participant);
    if (hourglassParticipant != nullptr)
    {
        const auto cParticipant = hourglassParticipant->Get();
        const auto cppParticipant = reinterpret_cast<SilKit::IParticipant*>(cParticipant);
        return dynamic_cast<SilKit::Core::IParticipantInternal&>(*cppParticipant);
    }

    throw std::bad_cast{};
}

} // namespace Tests
} // namespace SilKit
