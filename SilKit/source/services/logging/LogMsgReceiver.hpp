// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "Logger.hpp"

#include "IParticipantInternal.hpp"
#include "IMsgForLogMsgReceiver.hpp"
#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

class LogMsgReceiver
    : public IMsgForLogMsgReceiver
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    LogMsgReceiver(Core::IParticipantInternal* participant, Logger* logger);

public:
    void ReceiveMsg(const Core::IServiceEndpoint* /*from*/, const LogMsg& msg) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
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
} // namespace Services
} // namespace SilKit
