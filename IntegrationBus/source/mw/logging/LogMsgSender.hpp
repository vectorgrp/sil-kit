// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "IIbToLogMsgSender.hpp"
#include "IComAdapterInternal.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgSender
    : public IIbToLogMsgSender
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

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
};

} // namespace logging
} // namespace mw
} // namespace ib
