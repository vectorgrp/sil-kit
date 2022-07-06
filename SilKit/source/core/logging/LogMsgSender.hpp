// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IMsgForLogMsgSender.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

class LogMsgSender
    : public IMsgForLogMsgSender
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgSender(IParticipantInternal* participant);

public:
    void SendLogMsg(const LogMsg& msg);
    void SendLogMsg(LogMsg&& msg);

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor{};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
void LogMsgSender::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto LogMsgSender::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Logging
} // namespace Core
} // namespace SilKit
