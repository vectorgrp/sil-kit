// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ServiceDatatypes.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {
namespace service {

class ServiceDiscovery
    : public mw::IIbEndpoint<ServiceAnnouncement>
    , public mw::IIbSender<ServiceAnnouncement>
    , public IIbServiceEndpoint
{
public:
    ServiceDiscovery(IComAdapterInternal* comadapter, const std::string& participantName)
        : _comAdapter{comadapter}
        , _participantName{participantName}
    {
    }
    virtual ~ServiceDiscovery()
    {
    }
public: // Interfaces

    //IIbEndpoint
    inline void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)  override;
    inline auto EndpointAddress() const -> const ib::mw::EndpointAddress& override;
    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;
    inline void SendIbMessage(const ServiceAnnouncement& msg) const;
    inline void ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg) override;

private:
    IComAdapterInternal* _comAdapter{nullptr};
    std::string _participantName;
    ServiceId _serviceId;
};

// ================================================================================
//  Inline Implementations
// ================================================================================


void ServiceDiscovery::SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress)
{
    _serviceId.legacyEpa = endpointAddress;
}
auto ServiceDiscovery::EndpointAddress() const -> const ib::mw::EndpointAddress&
{
    return _serviceId.legacyEpa;
}

void ServiceDiscovery::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto ServiceDiscovery::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

void ServiceDiscovery::SendIbMessage(const ServiceAnnouncement& msg) const
{
    _comAdapter->SendIbMessage(this, msg);
}
void ServiceDiscovery::ReceiveIbMessage(const IIbServiceEndpoint* from, const ServiceAnnouncement& msg)
{
}
} // namespace service
} // namespace mw
} // namespace ib

