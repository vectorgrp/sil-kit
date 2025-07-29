// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <future>
#include <functional>
#include <set>

#include "IParticipantReplies.hpp"
#include "IParticipantInternal.hpp"
#include "IRequestReplyService.hpp"
#include "IRequestReplyProcedure.hpp"

#include "Uuid.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

class ParticipantReplies
    : public IParticipantReplies
    , public IRequestReplyProcedure
{
public:
    ParticipantReplies(IParticipantInternal* participant, IServiceEndpoint* requestReplyServiceEndpoint);

    // IParticipantReplies

    void CallAfterAllParticipantsReplied(std::function<void()> completionFunction) override;

    // IRequestReplyProcedure

    void ReceiveCall(IRequestReplyService* requestReplyService, Util::Uuid callUuid,
                     std::vector<uint8_t> callData) override;
    void ReceiveCallReturn(std::string fromParticipant, Util::Uuid callUuid, std::vector<uint8_t> callReturnData,
                           CallReturnStatus callReturnStatus) override;
    void SetRequestReplyServiceEndpoint(IServiceEndpoint* requestReplyService) override;


private:
    const FunctionType _functionType{FunctionType::ParticipantReplies};

    IParticipantInternal* _participant;
    IServiceEndpoint* _requestReplyServiceEndpoint;

    bool _barrierActive;
    std::set<std::string> _expectedParticipantsToSendCallReturns;
    std::function<void()> _completionFunction;
    Util::Uuid _activeUuid;
};


} // namespace RequestReply
} // namespace Core
} // namespace SilKit
