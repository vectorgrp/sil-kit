// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Logger.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbToLogMsgReceiver.hpp"
#include "IServiceId.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgReceiver
    : public IIbToLogMsgReceiver
    , public mw::IServiceId
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(IComAdapterInternal* comAdapter, Logger* logger);

public:
    void ReceiveIbMessage(const IServiceId* from, const LogMsg& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress &address) override;
    auto EndpointAddress(void) const -> const mw::EndpointAddress & override;
    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;


private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceId _serviceId{};
    
    logging::Logger* _logger;
};
// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgReceiver::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto LogMsgReceiver::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}


} // namespace logging
} // namespace mw
} // namespace ib
