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

class IRequestReplyProcedure;
class IParticipantReplies;

class IRequestReplyService
{
public:
    virtual ~IRequestReplyService() = default;

    virtual Util::Uuid Call(FunctionType functionType, std::vector<uint8_t> callData) = 0;
    virtual void SubmitCallReturn(Util::Uuid callUuid, FunctionType functionType, std::vector<uint8_t> callReturnData,
                                  CallReturnStatus callReturnStatus) = 0;
};

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
