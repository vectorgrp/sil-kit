// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantReplies.hpp"

#include "silkit/participant/exception.hpp"
#include "LoggerMessage.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

ParticipantReplies::ParticipantReplies(IParticipantInternal* participant, IServiceEndpoint* requestReplyServiceEndpoint)
    : _participant{participant}
    , _requestReplyServiceEndpoint{requestReplyServiceEndpoint}
    , _barrierActive{false}
    , _expectedParticipantsToSendCallReturns{}
    , _completionFunction{nullptr}
    , _activeUuid{}
{
}

// IParticipantReplies

void ParticipantReplies::CallAfterAllParticipantsReplied(std::function<void()> completionFunction)
{
    // Execute deferred to prevent additional participants to join between GetNumberOfRemoteReceivers and actual SendMsg in GetRequestReplyService()->Call()
    _participant->ExecuteDeferred([this, completionFunction = std::move(completionFunction)]() mutable {
        if (_barrierActive)
        {
            Services::Logging::Debug(_participant->GetLogger(),
                                     "Still waiting for replies from participants on a previous call, action when new "
                                     "call is replied will not be executed. This might lead to unexpected behavior.");
            return;
        }

        auto remoteReceivers =
            _participant->GetParticipantNamesOfRemoteReceivers(_requestReplyServiceEndpoint, "REQUESTREPLYCALL");
        _expectedParticipantsToSendCallReturns = std::set<std::string>(remoteReceivers.begin(), remoteReceivers.end());
        Services::Logging::Debug(_participant->GetLogger(), "Request replies of {} participant(s).",
                                 _expectedParticipantsToSendCallReturns.size());
        if (_expectedParticipantsToSendCallReturns.empty())
        {
            Services::Logging::Debug(_participant->GetLogger(),
                                     "Called CallAfterAllParticipantsReplied() with no other known participants.");
            completionFunction();
            return;
        }

        _barrierActive = true;
        _completionFunction = std::move(completionFunction);
        _activeUuid = _participant->GetRequestReplyService()->Call(_functionType, {});
    });
}

// IRequestReplyProcedure

void ParticipantReplies::SetRequestReplyServiceEndpoint(IServiceEndpoint* requestReplyServiceEndpoint)
{
    _requestReplyServiceEndpoint = requestReplyServiceEndpoint;
}

void ParticipantReplies::ReceiveCall(IRequestReplyService* requestReplyService, Util::Uuid callUuid,
                                     std::vector<uint8_t> /*callData*/)
{
    // Directly reply the call
    requestReplyService->SubmitCallReturn(callUuid, _functionType, {}, CallReturnStatus::Success);
}

void ParticipantReplies::ReceiveCallReturn(std::string fromParticipant, Util::Uuid callUuid,
                                           std::vector<uint8_t> /*callReturnData*/,
                                           CallReturnStatus /*callReturnStatus*/)
{
    // All values of callReturnStatus are ok to complete the request.
    // If we know the callUuid, collect call returns and call the completion function
    if (_barrierActive && callUuid == _activeUuid)
    {
        _expectedParticipantsToSendCallReturns.erase(fromParticipant);
        if (_expectedParticipantsToSendCallReturns.empty())
        {
            if (_completionFunction)
            {
                Services::Logging::Debug(_participant->GetLogger(), "Request participant replies completed.");
                _completionFunction();
            }
            _barrierActive = false;
        }
    }
}

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
