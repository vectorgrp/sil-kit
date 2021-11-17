// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Logger.hpp"

#include "IComAdapterInternal.hpp"
#include "IIbToLogMsgReceiver.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgReceiver
    : public IIbToLogMsgReceiver
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(IComAdapterInternal* comAdapter, Logger* logger);

public:
    void ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress &address) override;
    auto EndpointAddress(void) const -> const mw::EndpointAddress & override;

private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    
    logging::Logger* _logger;
};

} // namespace logging
} // namespace mw
} // namespace ib
