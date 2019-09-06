// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

#include "IIbToLogMsgDistributor.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgDistributor
    : public IIbToLogMsgDistributor
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgDistributor(IComAdapter* comAdapter);

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
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
};

} // namespace logging
} // namespace mw
} // namespace ib
