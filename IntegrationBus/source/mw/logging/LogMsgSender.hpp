// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "IIbToLogMsgSender.hpp"
#include "IParticipantInternal.hpp"
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
    LogMsgSender(IParticipantInternal* participant);

public:
    void SendLogMsg(const LogMsg& msg);
    void SendLogMsg(LogMsg&& msg);

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
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
