// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <memory>


namespace SilKit {
namespace Core {
struct IRegistryEventListener;
} // namespace Core
} // namespace SilKit


namespace SilKit {
namespace Config {
class IParticipantConfiguration;
} // namespace Config
} // namespace SilKit


namespace SilKit {
namespace Vendor {
namespace Vector {
class ISilKitRegistry;
} // namespace Vector
} // namespace Vendor
} // namespace SilKit


namespace VSilKit {


auto CreateSilKitRegistryWithDashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                                       SilKit::Core::IRegistryEventListener* registryEventListener)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>;


} // namespace VSilKit
