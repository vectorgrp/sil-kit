// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CreateSilKitRegistryImpl.hpp"

#include <memory>

#include "VAsioRegistry.hpp"


namespace SilKit {
namespace Vendor {
namespace Vector {

auto CreateSilKitRegistryImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>
{
    return std::make_unique<Core::VAsioRegistry>(config);
}

auto GetLoggerInternal(ISilKitRegistry* participant) -> SilKit::Services::Logging::ILoggerInternal*
{
    auto participantImpl = dynamic_cast<Core::VAsioRegistry*>(participant);
    if (participantImpl == nullptr)
    {
        throw SilKit::ConfigurationError("The provided participant is not a valid SilKit::Core::IParticipantInternal");
    }
    return participantImpl->GetLoggerInternal();
}

} // namespace Vector
} // namespace Vendor
} // namespace SilKit
