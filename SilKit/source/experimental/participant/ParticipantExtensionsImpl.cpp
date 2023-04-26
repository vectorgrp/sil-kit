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

} // namespace Participant
} // namespace Experimental
} // namespace SilKit
