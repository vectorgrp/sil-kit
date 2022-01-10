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
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgSender::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LogMsgSender::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace logging
} // namespace mw
} // namespace ib
