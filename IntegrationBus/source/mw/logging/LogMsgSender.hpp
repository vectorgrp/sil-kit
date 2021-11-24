// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "IIbToLogMsgSender.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgSender
    : public IIbToLogMsgSender
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgSender(IComAdapterInternal* comAdapter);

public:
    void SendLogMsg(const LogMsg& msg);
    void SendLogMsg(LogMsg&& msg);

    void SetEndpointAddress(const mw::EndpointAddress &address) override;
    auto EndpointAddress(void) const -> const mw::EndpointAddress & override;

    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceId _serviceId{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgSender::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto LogMsgSender::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

} // namespace logging
} // namespace mw
} // namespace ib
