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

#include "ParticipantReplies.hpp"

#include "silkit/participant/exception.hpp"
#include "ILogger.hpp"

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
            Services::Logging::Debug(_participant->GetLogger(), "Only one barrier can be active at a time, ignoring.");
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

void ParticipantReplies::ReceiveCallReturn(std::string fromParticipant, Util::Uuid callUuid, std::vector<uint8_t> /*callReturnData*/, CallReturnStatus /*callReturnStatus*/)
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

