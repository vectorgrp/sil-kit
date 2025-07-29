// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <vector>

namespace SilKit {
namespace Core {
namespace RequestReply {

// Methods of ParticipantReplies used outside of RequestReplyService
class IParticipantReplies
{
public:
    virtual ~IParticipantReplies() = default;

    //! Send a RequestReplyCall to all participants. They will immediately respond with a RequestReplyCallReturn.
    //! If all replies arrived, trigger the completionFunction.
    virtual void CallAfterAllParticipantsReplied(std::function<void()> completionFunction) = 0;
};

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
