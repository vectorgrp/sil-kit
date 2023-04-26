/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
