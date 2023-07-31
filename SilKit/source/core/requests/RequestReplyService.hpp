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

#pragma once

#include <functional>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <atomic>
#include <unordered_set>
#include <future>

#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IRequestReplyService.hpp"
#include "IRequestReplyProcedure.hpp"

#include "Uuid.hpp"
#include "procs/ParticipantReplies.hpp"

namespace SilKit {
namespace Core {
namespace RequestReply {

using ProcedureMap = std::unordered_map<FunctionType, IRequestReplyProcedure*>;

class RequestReplyService
    : public Core::IReceiver<RequestReplyCall, RequestReplyCallReturn>
    , public Core::ISender<RequestReplyCall, RequestReplyCallReturn>
    , public IServiceEndpoint
    , public IRequestReplyService
{
public: 

    RequestReplyService(IParticipantInternal* participant, const std::string& participantName, ProcedureMap procedures);
    virtual ~RequestReplyService() noexcept;
  
    // Disconnect handler
    void OnParticpantRemoval(const std::string& participantName);

    // IRequestReplyService
    Util::Uuid Call(FunctionType functionType, std::vector<uint8_t> callData) override;
    void SubmitCallReturn(Util::Uuid callUuid, FunctionType functionType, std::vector<uint8_t> callReturnData, CallReturnStatus callReturnStatus) override;

    // IServiceEndpoint
    void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

    // IReceiver
    void ReceiveMsg(const IServiceEndpoint* from, const RequestReplyCall& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const RequestReplyCallReturn& msg) override;

private:

    void RemovePartcipantFromDisconnectLookup(Util::Uuid callUuid, const std::string& participantName);
    void ForwardCallToProcedure(const RequestReplyCall& msg);
    void ForwardCallReturnToProcedure(std::string fromParticipant, const RequestReplyCallReturn& msg);
    
private:
    IParticipantInternal* _participant{nullptr};
    std::string _participantName;
    ServiceDescriptor _serviceDescriptor;
    
    mutable std::recursive_mutex _requestReplyMx;
    std::atomic<bool> _shuttingDown{false};

    std::map<std::string, std::map<Util::Uuid, RequestReplyCallReturn>> _participantDisconnectCallReturns;

    std::map<Util::Uuid, std::string> _requestReplyInitiatorByCallUuid;

    ProcedureMap _procedures;
};

} // namespace RequestReply
} // namespace Core
} // namespace SilKit

