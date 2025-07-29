// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <vector>

#include "RequestReplyDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

class IRequestReplyService;

// Methods of prodecure classes (e.g. ParticipantReplies) used in RequestReplyService
class IRequestReplyProcedure
{
public:
    virtual ~IRequestReplyProcedure() = default;

    virtual void ReceiveCall(IRequestReplyService* requestReplyService, Util::Uuid callUuid,
                             std::vector<uint8_t> callData) = 0;
    virtual void ReceiveCallReturn(std::string fromParticipant, Util::Uuid callUuid,
                                   std::vector<uint8_t> callReturnData, CallReturnStatus callReturnStatus) = 0;
    virtual void SetRequestReplyServiceEndpoint(IServiceEndpoint* requestReplyServiceEndpoint) = 0;
};

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
