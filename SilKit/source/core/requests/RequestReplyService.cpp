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

#include "RequestReplyService.hpp"

#include "silkit/participant/exception.hpp"

using namespace std::chrono_literals;
namespace SilKit {
namespace Core {
namespace RequestReply {

RequestReplyService::RequestReplyService(IParticipantInternal* participant, const std::string& participantName, ProcedureMap prodecures)
    : _participant{participant}
    , _participantName{participantName}
    , _participantDisconnectCallReturns{}
    , _procedures{prodecures}
{
    for (auto&& p : _procedures)
    {
        p.second->SetRequestReplyServiceEndpoint(this);
    }
}

RequestReplyService::~RequestReplyService() noexcept
{
    _shuttingDown = true;
}

// IRequestReplyService

Util::Uuid RequestReplyService::Call(FunctionType functionType, std::vector<uint8_t> callData)
{
    if (functionType == FunctionType::Invalid)
    {
        throw SilKitError("RequestReplyService::Call(): FunctionType::Invalid not allowed");
    }

    std::unique_lock<decltype(_requestReplyMx)> lock(_requestReplyMx);

    const auto callUuid = Util::Uuid::GenerateRandom();

    // Get the participant names that receive the RequestReplyCall.
    // This is needed to locally trigger the RequestReplyCallReturn if a participant disconnects in between.
    auto receivingParticipants =
        _participant->GetParticipantNamesOfRemoteReceivers(this, "REQUESTREPLYCALL");

    // Prepare RequestReplyCallReturn message for disconnects. There, we only have the name
    // of the disconnected participant, so we directly map name and reply
    for (auto&& name : receivingParticipants)
    {
        RequestReplyCallReturn disconnectMsg;
        disconnectMsg.callUuid = callUuid;
        disconnectMsg.functionType = functionType;
        disconnectMsg.callReturnData = {};
        disconnectMsg.callReturnStatus = CallReturnStatus::RecipientDisconnected;
        _participantDisconnectCallReturns[name].emplace(callUuid, disconnectMsg);
    }

    // Send RequestReplyCall
    RequestReplyCall call{};
    call.callUuid = callUuid;
    call.functionType = functionType;
    call.callData = std::move(callData);
    _participant->SendMsg(this, std::move(call));

    return callUuid;
}

void RequestReplyService::SubmitCallReturn(Util::Uuid callUuid, FunctionType functionType,
                                           std::vector<uint8_t> callReturnData, CallReturnStatus callReturnStatus)
{
    if (functionType == FunctionType::Invalid)
    {
        throw SilKitError("RequestReplyService::SubmitCallReturn(): FunctionType::Invalid not allowed");
    }

    auto it = _requestReplyInitiatorByCallUuid.find(callUuid);
    if (it == _requestReplyInitiatorByCallUuid.end())
    {
        throw SilKitError("RequestReplyService::SubmitCallReturn(): callUuid unknown");
    }
    const auto& initiator = (*it).second;

    RequestReplyCallReturn callReturn{};
    callReturn.callUuid = callUuid;
    callReturn.functionType = functionType;
    callReturn.callReturnData = std::move(callReturnData);
    callReturn.callReturnStatus = callReturnStatus;
    _participant->SendMsg(this, initiator, std::move(callReturn));
}


// IReceiver

void RequestReplyService::ReceiveMsg(const IServiceEndpoint* from, const RequestReplyCall& msg)
{
    // Remember the initiator to send back the reply only to that participant
    auto it = _requestReplyInitiatorByCallUuid.find(msg.callUuid);
    if (it != _requestReplyInitiatorByCallUuid.end())
    {
        throw SilKitError("RequestReplyService: Received duplicate callUuid.");
    }
    _requestReplyInitiatorByCallUuid.emplace(msg.callUuid, from->GetServiceDescriptor().GetParticipantName());
    ForwardCallToProcedure(msg);
}

void RequestReplyService::ReceiveMsg(const IServiceEndpoint* from, const RequestReplyCallReturn& msg)
{
    auto fromParticipant = from->GetServiceDescriptor().GetParticipantName();
    RemovePartcipantFromDisconnectLookup(msg.callUuid, fromParticipant);
    ForwardCallReturnToProcedure(fromParticipant, msg);
}

// DisconnectLookup bookkeeping

void RequestReplyService::RemovePartcipantFromDisconnectLookup(Util::Uuid callUuid, const std::string& participantName)
{
    std::unique_lock<decltype(_requestReplyMx)> lock(_requestReplyMx);

    auto itParticipant = _participantDisconnectCallReturns.find(participantName);
    if (itParticipant != _participantDisconnectCallReturns.end())
    {
        auto callReturnMsgByCallUuid = (*itParticipant).second;
        auto it = callReturnMsgByCallUuid.find(callUuid);
        if (it != callReturnMsgByCallUuid.end())
        {
            callReturnMsgByCallUuid.erase(it);
        }
        if (callReturnMsgByCallUuid.empty())
        {
            _participantDisconnectCallReturns.erase(itParticipant);
        }
    }
}

// Disconnect

void RequestReplyService::OnParticpantRemoval(const std::string& participantName)
{
    std::unique_lock<decltype(_requestReplyMx)> lock(_requestReplyMx);

    // Trigger call return on disconnect
    auto callIt = _participantDisconnectCallReturns.find(participantName);
    if (callIt != _participantDisconnectCallReturns.end())
    {
        for (auto&& msg : (*callIt).second)
        {
            ForwardCallReturnToProcedure(participantName, msg.second);
        }
        _participantDisconnectCallReturns.erase(callIt);
    }
}

// Prodecure dependencies

void RequestReplyService::ForwardCallToProcedure(const RequestReplyCall& msg)
{
    auto it = _procedures.find(msg.functionType);
    if (it == _procedures.end())
    {
        SubmitCallReturn(msg.callUuid, msg.functionType, {}, CallReturnStatus::UnknownFunctionType);
    }
    else
    {
        try
        {
            it->second->ReceiveCall(this, msg.callUuid, msg.callData);
        }
        catch (const std::exception& /*e*/)
        {
            SubmitCallReturn(msg.callUuid, msg.functionType, {}, CallReturnStatus::ProcedureError);
        }
    }
}

void RequestReplyService::ForwardCallReturnToProcedure(std::string fromParticipant, const RequestReplyCallReturn& msg)
{
    auto it = _procedures.find(msg.functionType);
    if (it == _procedures.end())
    {
        throw SilKitError("RequestReplyService::ForwardCallReturnToProcedure(): FunctionType unknown");
    }
    it->second->ReceiveCallReturn(fromParticipant, msg.callUuid, msg.callReturnData, msg.callReturnStatus);
}

// IServiceEndpoint

void RequestReplyService::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto RequestReplyService::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace RequestReply
} // namespace Core
} // namespace SilKit
