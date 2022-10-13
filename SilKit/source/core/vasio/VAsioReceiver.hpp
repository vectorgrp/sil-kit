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

#include "SilKitLink.hpp"

#include "VAsioDatatypes.hpp"

#include "MessageTracing.hpp"
#include "IServiceEndpoint.hpp"
#include "SerializedMessage.hpp"

namespace SilKit {
namespace Core {

struct RemoteServiceEndpoint : IServiceEndpoint
{
    void SetServiceDescriptor(const SilKit::Core::ServiceDescriptor&) override 
    {
        throw LogicError("This method is not supposed to be used in this struct.");
    }

    auto GetServiceDescriptor() const -> const ServiceDescriptor & override
    {
        return _serviceDescriptor; 
    }

    RemoteServiceEndpoint(const ServiceDescriptor& descriptor)
    {
        _serviceDescriptor = descriptor;
    }

private:
    ServiceDescriptor _serviceDescriptor;
};

class MessageBuffer;

class IVAsioReceiver
{
public:
    // ----------------------------------------
    // Public interface methods
    virtual ~IVAsioReceiver() = default;
    virtual auto GetDescriptor() const -> const VAsioMsgSubscriber& = 0;
    virtual void ReceiveRawMsg(IVAsioPeer* from, const ServiceDescriptor& descriptor, SerializedMessage&& buffer) = 0;
};

template <class MsgT>
class VAsioReceiver
    : public IVAsioReceiver
    , public IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<SilKitLink<MsgT>> link, Services::Logging::ILogger* logger);

public:
    // ----------------------------------------
    // Public interface methods
    auto GetDescriptor() const -> const VAsioMsgSubscriber& override;
    void ReceiveRawMsg(IVAsioPeer* from, const ServiceDescriptor& descriptor, SerializedMessage&& buffer) override;
    void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) override
    {
        _serviceDescriptor = serviceDescriptor;
    }
    auto GetServiceDescriptor() const -> const ServiceDescriptor& override
    {
        return _serviceDescriptor;
    }

private:
    // ----------------------------------------
    // private members
    VAsioMsgSubscriber _subscriptionInfo;
    std::shared_ptr<SilKitLink<MsgT>> _link;
    Services::Logging::ILogger* _logger;
    ServiceDescriptor _serviceDescriptor;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template <class MsgT>
VAsioReceiver<MsgT>::VAsioReceiver(VAsioMsgSubscriber subscriberInfo, std::shared_ptr<SilKitLink<MsgT>> link, Services::Logging::ILogger* logger)
    : _subscriptionInfo{std::move(subscriberInfo)}
    , _link{link}
    , _logger{logger}
{
    _serviceDescriptor.SetNetworkName(_subscriptionInfo.networkName);
}

template <class MsgT>
auto VAsioReceiver<MsgT>::GetDescriptor() const -> const VAsioMsgSubscriber&
{
    return _subscriptionInfo;
}

template <class MsgT>
void VAsioReceiver<MsgT>::ReceiveRawMsg(IVAsioPeer* /*from*/, const ServiceDescriptor& descriptor, SerializedMessage&& buffer)
{
    MsgT msg = buffer.Deserialize<MsgT>();

    Services::TraceRx(_logger, this, msg, descriptor);

    auto remoteId = RemoteServiceEndpoint(descriptor);
    _link->DistributeRemoteSilKitMessage(&remoteId, std::move(msg));

}

} // namespace Core
} // namespace SilKit
