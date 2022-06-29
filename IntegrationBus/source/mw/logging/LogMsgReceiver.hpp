// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Logger.hpp"

#include "IParticipantInternal.hpp"
#include "IIbToLogMsgReceiver.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace mw {
namespace logging {

class LogMsgReceiver
    : public IIbToLogMsgReceiver
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(IParticipantInternal* participant, Logger* logger);

public:
    void ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const LogMsg& msg) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    
    logging::Logger* _logger;
};
// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgReceiver::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LogMsgReceiver::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace logging
} // namespace mw
} // namespace ib
