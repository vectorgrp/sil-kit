// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include "silkit/capi/SilKit.h"

#include "silkit/detail/impl/config/ParticipantConfiguration.hpp"
#include "silkit/detail/impl/vendor/SilKitRegistry.hpp"

#include "silkit/SilKitMacros.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Vendor {
namespace Vector {

auto CreateSilKitRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>
{
    auto& config = dynamic_cast<Impl::Config::ParticipantConfiguration&>(*participantConfig.get());

    SilKit_Vendor_Vector_SilKitRegistry* silKitRegistry{nullptr};

    const auto returnCode = SilKit_Vendor_Vector_SilKitRegistry_Create(&silKitRegistry, config.Get());
    Impl::ThrowOnError(returnCode);

    return std::make_unique<Impl::Vendor::Vector::SilKitRegistry>(silKitRegistry);
}

} // namespace Vector
} // namespace Vendor
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


namespace SilKit {
namespace Vendor {
namespace Vector {
using SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Vendor::Vector::CreateSilKitRegistry;
} // namespace Vector
} // namespace Vendor
} // namespace SilKit
