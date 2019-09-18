// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

#include "IIbToLogMsgReceiver.hpp"
#include "Logger.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgReceiver
    : public IIbToLogMsgReceiver
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(IComAdapter* comAdapter);

public:
    void SetLogger(Logger* logger);

    void ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress &address) override;
    auto EndpointAddress(void) const -> const mw::EndpointAddress & override;

private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    
    logging::Logger* _logger;
};

} // namespace logging
} // namespace mw
} // namespace ib
