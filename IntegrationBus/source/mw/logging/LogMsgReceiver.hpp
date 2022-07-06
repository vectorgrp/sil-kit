// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Logger.hpp"

#include "IParticipantInternal.hpp"
#include "IMsgForLogMsgReceiver.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

class LogMsgReceiver
    : public IMsgForLogMsgReceiver
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(IParticipantInternal* participant, Logger* logger);

public:
    void ReceiveSilKitMessage(const IServiceEndpoint* /*from*/, const LogMsg& msg) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
    
    Logging::Logger* _logger;
};
// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgReceiver::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LogMsgReceiver::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Logging
} // namespace Core
} // namespace SilKit
