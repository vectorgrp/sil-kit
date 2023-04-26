// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "IParticipantInternal.hpp"


namespace SilKit {
namespace Tests {

inline auto ToParticipantInternal(SilKit::IParticipant &participant) -> SilKit::Core::IParticipantInternal &
{
    // Try casting directly into IParticipantInternal, will result in nullptr for hourglass-participants.
    auto *participantInternal = dynamic_cast<SilKit::Core::IParticipantInternal *>(&participant);
    if (participantInternal != nullptr)
    {
        return *participantInternal;
    }

    using HourglassParticipant = SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Participant;

    // Try casting into the concrete hourglass participant, then get the C-API pointer (which is the internal
    // IParticipant pointer), then try casting to IParticipantInternal.
    auto *hourglassParticipant = dynamic_cast<HourglassParticipant *>(&participant);
    if (hourglassParticipant != nullptr)
    {
        const auto cParticipant = hourglassParticipant->Get();
        const auto cppParticipant = reinterpret_cast<SilKit::IParticipant *>(cParticipant);
        return dynamic_cast<SilKit::Core::IParticipantInternal &>(*cppParticipant);
    }

    throw std::bad_cast{};
}

} // namespace Tests
} // namespace SilKit
