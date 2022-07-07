// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
        throw std::logic_error("This method is not supposed to be used in this struct."); 
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

    Services::TraceRx(_logger, this, msg);

    auto remoteId = RemoteServiceEndpoint(descriptor);
    _link->DistributeRemoteSilKitMessage(&remoteId, std::move(msg));

}

} // namespace Core
} // namespace SilKit
